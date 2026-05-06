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
#include "llvm/Plugins/PassPlugin.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include "llvm/Support/raw_ostream.h"
#include <llvm/TargetParser/Host.h>
#include "llvm/Demangle/Demangle.h"

#include <memory>
#include <vector>
#include <iostream>
#include <algorithm>
#include <map>

extern llvm::Module* Module;
extern llvm::LLVMContext* Context;

extern llvm::Type* void_t;// void
extern llvm::Type* ptr_t;// void*
extern llvm::Type* i1_t;// bool
extern llvm::Type* i8_t;// char
extern llvm::Type* i16_t;// short
extern llvm::Type* i32_t;// int
extern llvm::Type* i64_t;// long
extern llvm::Type* f16_t;// half
extern llvm::Type* f32_t;// float
extern llvm::Type* f64_t;// double

extern llvm::Function* printChar;
extern llvm::Function* printStr;
//extern llvm::Function* printBool;
extern llvm::Function* printUInt;
extern llvm::Function* printUInt64;
extern llvm::Function* printFloat;
extern llvm::Function* printDouble;
extern llvm::Function* printlnChar;

void populateGlobals(llvm::Module& _Module);
void populateGlobals(llvm::Function& F);
void populateGlobals();

void populateStdLib(llvm::Function& F);

std::string valueToString(llvm::Value* inst);
void printFuncSig(const llvm::Function& F);

llvm::GlobalVariable* createGlobalString(std::string str, std::string varName="str");
llvm::GlobalVariable* createGlobalPtrArray(llvm::ArrayRef<llvm::Constant*> vals, std::string varName);
llvm::GlobalVariable* createGlobalInt(int val, std::string varName);
llvm::GlobalVariable* createGlobalIntArray(llvm::ArrayRef<llvm::Constant*> vals, std::string varName);

llvm::CallInst* doCall(llvm::Function* f, llvm::Value* val, llvm::BasicBlock::iterator beforeInst);
llvm::CallInst* doCall(llvm::Function* f, char chr, llvm::BasicBlock::iterator beforeInst);

std::string basicGetTypeAsString(llvm::Type* ty);
extern std::vector<llvm::Function*> visitedFunctions_global;
extern std::map<llvm::Value*, std::string> determinedTypes;
std::string getTypeAsString(llvm::Value* val, std::vector<llvm::Function*>& visitedFunctions=visitedFunctions_global, unsigned int depth=0, std::string indent="    ");
int getTypeBitWidth(llvm::Type* ty);
//std::string attemptFindPointerType(llvm::Value* val, std::vector<llvm::Function*>& visitedFunctions=visitedFunctions_global, unsigned int depth=0, std::string indent="    ");

#endif// __LLVM_HELPERS