#ifndef __LLVM_HELPERS
#define __LLVM_HELPERS

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/Support/RandomNumberGenerator.h"
#include "llvm/Pass.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"

extern llvm::Module* Module;
extern llvm::LLVMContext* Context;

extern llvm::Type* ptr_t;
extern llvm::Type* char_t;

extern llvm::Function* printChar;
extern llvm::Function* printStr;
//extern llvm::Function* printBool;
extern llvm::Function* printUInt;
extern llvm::Function* printUInt64;
extern llvm::Function* printFloat;
extern llvm::Function* printDouble;
extern llvm::Function* printlnChar;

void populateGlobals(llvm::Function& F);

llvm::GlobalVariable* createGlobalString(std::string str);

llvm::CallInst* doCall(llvm::Function* f, llvm::Value* val, llvm::BasicBlock::iterator beforeInst);
llvm::CallInst* doCall(llvm::Function* f, char chr, llvm::BasicBlock::iterator beforeInst);

std::string getTypeString(llvm::Type* ty);
llvm::CallInst* tryPrintValue(llvm::Value* val, llvm::BasicBlock::iterator beforeInstr);

#endif// __LLVM_HELPERS