#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include <set>
#include <queue>

using namespace llvm;
using namespace std;

#define DEBUG_TYPE "ConstantPropagation"

namespace
{

  struct ConstantPropagation : public FunctionPass
  {
    static char ID;
    ConstantPropagation() : FunctionPass(ID) {}
    // function to get LHS Value from an LLVM instruction
    std::string getLhsVar(const llvm::Instruction &ins)
    {
      std::string instrStr;
      llvm::raw_string_ostream rso(instrStr);
      ins.print(rso);
      rso.flush();
      std::string lhsVar;
      std::istringstream stream(instrStr);
      stream >> lhsVar;
      return lhsVar;
    }
    // splitting the string
    std::string getLhsVarFromValue(Value *ptrOperand)
    {
      std::string ptrName;
      llvm::raw_string_ostream rso(ptrName);
      ptrOperand->printAsOperand(rso, false); // `false` omits the type information
      rso.flush();
      return ptrName;
    }
    // function to print constants with line numbers
    void printConstants(map<string, int> &insToVal, map<string, int> &insToLine)
    {

      map<int, int> lineToVal;
      for (auto mp1 : insToVal)
      {
        lineToVal[insToLine[mp1.first]] = mp1.second;
      }
      for (auto mp2 : lineToVal)
      {
        // printing only the constants
        if (mp2.second != INT_MIN && mp2.second != INT_MAX)
        {
          errs() << mp2.first << ":" << mp2.second << "\n";
        }
      }
    }

    bool compareMaps(const map<string, int> &map1, const map<string, int> &map2)
    {
      bool mapsEqual = true;

      // Check if each key-value pair in map1 exists in map2 and matches
      for (const auto &pair : map1)
      {
        auto it = map2.find(pair.first);
        if (it == map2.end() || it->second != pair.second)
        {
          mapsEqual = false;
        }
      }

      // Check if map2 has any additional keys not in map1
      for (const auto &pair : map2)
      {
        if (map1.find(pair.first) == map1.end())
        {
          mapsEqual = false;
        }
      }

      // Return the overall comparison result
      return mapsEqual;
    }
    bool runOnFunction(Function &F) override
    {

      // Map to hold constant propagation information with nullptr as "no value" indicator
      std::map<BasicBlock *, std::map<string, int>> IN, OUT;
      map<string, int> insToLine;
      std::map<string, int> outM;
      int line = 0;
      for (auto &BB : F)
      {
        for (auto &ins : BB)
        {
          ++line;
          std::string lhsVar;
          lhsVar = getLhsVar(ins);
          // assigning line numbers to variables
          if (insToLine.find(lhsVar) == insToLine.end())
          {
            insToLine[lhsVar] = line;
          }
          // Perform all type checks in a single if condition
          if (auto *loadInst = dyn_cast<LoadInst>(&ins))
          {
            outM[lhsVar] = INT_MAX;
          }
          else if (isa<AllocaInst>(ins))
          {
            outM[lhsVar] = INT_MAX;
          }
          else if (isa<BinaryOperator>(ins))
          {
            outM[lhsVar] = INT_MAX;
          }
          else if (auto *cmpInst = dyn_cast<ICmpInst>(&ins))
          {
            outM[lhsVar] = INT_MAX;
          }
        }
      }
      // initializing in and out for every basic block
      for (auto &BB : F)
      {
        OUT[&BB] = outM;
        IN[&BB] = outM;
      }

      BasicBlock &startBlock = F.getEntryBlock();
      for (auto mp : OUT[&startBlock])
      {
        IN[&startBlock][mp.first] = INT_MIN;
      }
      // worklist algorithm to process blocks
      queue<BasicBlock *> q;
      q.push(&startBlock);
      while (!q.empty())
      {
        BasicBlock *block = q.front();
        q.pop();

        map<string, int> inMapTemp, outMapTemp;
        inMapTemp = IN[block];
        // meet operator
        for (auto predBB : predecessors(block))
        {

          for (auto mp : OUT[predBB])
          { // checking if one of the values is not a constant
            if (inMapTemp[mp.first] == INT_MIN || mp.second == INT_MIN)
            {
              inMapTemp[mp.first] = INT_MIN;
            }
            else if (inMapTemp[mp.first] == INT_MAX)
            {
              // the variable is only coming from one of the blocks at this point
              inMapTemp[mp.first] = mp.second;
            }
            else if (mp.second == INT_MAX)
            {
            }
            else
            {
              if (inMapTemp[mp.first] != mp.second)
              {
                inMapTemp[mp.first] = INT_MIN;
              }
            }
          }
        }

        IN[block] = inMapTemp;
        outMapTemp = inMapTemp;
        // processing every instruction in the block
        for (auto &ins : *block)
        {
          string lhsVar = getLhsVar(ins);

          // Handle Store Instruction
          if (ins.getOpcode() == Instruction::Store)
          {
            StoreInst *storeInst = cast<StoreInst>(&ins);
            Value *value = storeInst->getValueOperand();
            auto *constValue = dyn_cast<ConstantInt>(value);
            Value *ptr = storeInst->getPointerOperand();
            if (constValue)
            {
              Value *ptr = storeInst->getPointerOperand();
              std::string ptrName;
              ptrName = getLhsVarFromValue(ptr);
              outMapTemp[ptrName] = constValue->getZExtValue();
            }
            else
            {
              string ptrVarName = getLhsVarFromValue(ptr);
              string valueVarName = getLhsVarFromValue(value);
              outMapTemp[ptrVarName] = outMapTemp[valueVarName];
            }
          }

          // Handle Load Instruction
          else if (ins.getOpcode() == Instruction::Load)
          {
            LoadInst *loadInst = cast<LoadInst>(&ins);
            Value *ptr = loadInst->getPointerOperand();
            string lhsVar = getLhsVar(ins);
            string rhsVar = getLhsVarFromValue(ptr);
            outMapTemp[lhsVar] = outMapTemp[rhsVar];
          }

          // Handle Binary Operations
          if (ins.isBinaryOp())
          {
            Value *lhs = ins.getOperand(0); // Left operand
            Value *rhs = ins.getOperand(1); // Right operand
            string operand1 = getLhsVarFromValue(lhs);
            string operand2 = getLhsVarFromValue(rhs);
            string binaryVar = getLhsVar(ins);

            auto *binaryInst = cast<BinaryOperator>(&ins);
            int lhsval, rhsval;

            // Retrieve operand values
            if (auto *lhsConst = dyn_cast<ConstantInt>(lhs))
            {
              lhsval = lhsConst->getZExtValue();
            }
            else
            {
              lhsval = outMapTemp[operand1];
            }

            if (auto *rhsConst = dyn_cast<ConstantInt>(rhs))
            {
              rhsval = rhsConst->getZExtValue();
            }
            else
            {
              rhsval = outMapTemp[operand2];
            }
            // computing binary operation result
            int computedVal = 0;
            switch (binaryInst->getOpcode())
            {
            case Instruction::Add:
              computedVal = lhsval + rhsval;
              break;
            case Instruction::Sub:
              computedVal = lhsval - rhsval;
              break;
            case Instruction::Mul:
              computedVal = lhsval * rhsval;
              break;
            case Instruction::SDiv: // Signed division
              if (rhsval != 0)
              {
                computedVal = lhsval / rhsval;
              }
              else
              {

                computedVal = 0; // Or handle differently (e.g., error handling or setting to NaN)
              }
              break;
            default:

              computedVal = 0;
            }

            if (lhsval == INT_MIN || rhsval == INT_MIN)
            {

              computedVal = INT_MIN;
            }
            // updating the out values
            outMapTemp[binaryVar] = computedVal;
          }

          if (auto *cmpInst = dyn_cast<ICmpInst>(&ins))
          {
            // Operands of the comparison
            Value *lhs = cmpInst->getOperand(0); // Left-hand side operand
            Value *rhs = cmpInst->getOperand(1); // Right-hand side operand
            string operand1 = getLhsVarFromValue(lhs);
            string operand2 = getLhsVarFromValue(rhs);

            int lhsVal, rhsVal;
            // computing the value of the left hand side

            if (auto *lhsConst = dyn_cast<ConstantInt>(lhs))
            {
              lhsVal = lhsConst->getZExtValue();
            }
            else
            {
              string operand1 = getLhsVarFromValue(lhs);
              lhsVal = outMapTemp[operand1];
            }
            // computing value of right hand side operands
            if (auto *rhsConst = dyn_cast<ConstantInt>(rhs))
            {
              rhsVal = rhsConst->getZExtValue();
            }
            else
            {
              string operand2 = getLhsVarFromValue(rhs);
              rhsVal = outMapTemp[operand2];
            }
            // handling cases when one of the values is not a constant
            ICmpInst::Predicate predicate = cmpInst->getPredicate();
            if (rhsVal == INT_MIN || lhsVal == INT_MIN)
            {
              string lhsVarName = getLhsVar(ins);
              outMapTemp[lhsVarName] = 2;
            }
            // Now check if it's followed by a branch instruction (conditional)
            if (auto *brInst = dyn_cast<BranchInst>(cmpInst->getNextNode()))
            {
              if (brInst->isConditional())
              {
                string lhsVarName = getLhsVar(ins);

                if (rhsVal == INT_MIN || lhsVal == INT_MIN)
                {
                  string lhsVarName = getLhsVar(ins);
                  outMapTemp[lhsVarName] = 2;
                }
                else
                {
                  // computing the predicate
                  bool condition = false;
                  switch (predicate)
                  {
                  case ICmpInst::ICMP_EQ:
                    condition = (lhsVal == rhsVal);
                    break;
                  case ICmpInst::ICMP_NE:
                    condition = (lhsVal != rhsVal);
                    break;
                  case ICmpInst::ICMP_SGT:
                    condition = (lhsVal > rhsVal);
                    break;
                  case ICmpInst::ICMP_SLT:
                    condition = (lhsVal < rhsVal);
                    break;
                  case ICmpInst::ICMP_SGE:
                    condition = (lhsVal >= rhsVal);
                    break;
                  case ICmpInst::ICMP_SLE:
                    condition = (lhsVal <= rhsVal);
                    break;
                  default:

                    break;
                  }
                  outMapTemp[lhsVarName] = condition;
                }
              }
            }
          }
          else if (auto *brInst = dyn_cast<BranchInst>(&ins))
          {
            if (brInst->isConditional())
            {
              // Conditional branch: It has two successors
              BasicBlock *trueBlock = brInst->getSuccessor(0);  // Block to jump to if condition is true
              BasicBlock *falseBlock = brInst->getSuccessor(1); // Block to jump to if condition is false
              Value *cond = brInst->getCondition();
              string condlhsVar = getLhsVarFromValue(cond);
              int condStateVal = outMapTemp[condlhsVar];
              outMapTemp.erase(condlhsVar);
              bool mapsEqual = compareMaps(OUT[block], outMapTemp);
              if (!mapsEqual)
              {
                if (condStateVal == 1)
                {
                  q.push(trueBlock);
                }
                else if (condStateVal == 0)
                {
                  q.push(falseBlock);
                }
                else
                {
                  q.push(trueBlock);
                  q.push(falseBlock);
                }
              }
            }
            else
            {
              // Unconditional branch: Only one successor
              BasicBlock *nextBlock = brInst->getSuccessor(0);      // The only successor for an unconditional branch
              bool mapsEqual = compareMaps(OUT[block], outMapTemp); // checking if there is change in the value of one of the variables in the block
              if (!mapsEqual)
              {
                q.push(nextBlock);
              } // Add the next block to the queue
            }

            OUT[block] = outMapTemp;
          }
          else if (auto *returnInst = dyn_cast<ReturnInst>(&ins))
          {
            OUT[block] = outMapTemp;
          }
        }
      }
      // printing the constants for each basic block at the end of the program
      for (auto &BB : F)
      {
        errs() << "-----" << BB.getName() << "-----\n";
        printConstants(OUT[&BB], insToLine);
      }
      for (auto &BB : F)
      {
        for (auto it = BB.begin(); it != BB.end();)
        {
          Instruction &inst = *it++;
          std::string lhsVar = getLhsVar(inst);

          // Check if the variable has a constant value in the OUT map
          for (auto &outMap : OUT[&BB])
          {
            if (outMap.first == lhsVar && outMap.second != INT_MIN && outMap.second != INT_MAX)
            {
              errs() << "Replacing uses of instruction: " << inst << " with constant value: " << outMap.second << "\n";

              // Replace all uses of the instruction with the constant value
              Constant *constVal = ConstantInt::get(inst.getType(), outMap.second);
              inst.replaceAllUsesWith(constVal);

              // Erase the instruction
              inst.eraseFromParent();
              break;
            }
          }
        }
      }

      // Printing updated function IR after constant propagation and instruction removal
      errs() << "Updated function after constant propagation:\n";
      F.print(errs());
      return true;
    }
  };

}

char ConstantPropagation::ID = 0;
static RegisterPass<ConstantPropagation> X("ConstantPropagation", "",
                                           false /* Only looks at CFG */,
                                           false /* Analysis Pass */);
