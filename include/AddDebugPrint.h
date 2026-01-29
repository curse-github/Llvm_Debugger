#ifndef __ADD_DEBUG_PRINT
#define __ADD_DEBUG_PRINT
#ifdef _WIN32
    #ifdef _BUILD_ADD_DBG_PRNT
        #define ADD_DBG_PRNT_API __declspec(dllexport)
    #else
        #define ADD_DBG_PRNT_API __declspec(dllimport)
    #endif
#else
    #define ADD_DBG_PRNT_API
#endif

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

#include <memory>
#include <random>
#include <vector>

struct ADD_DBG_PRNT_API AddDebugPrint : public llvm::PassInfoMixin<AddDebugPrint> {
    static bool isRequired() { return true; }

    llvm::PreservedAnalyses run(llvm::Function& F, llvm::FunctionAnalysisManager&);
    void addDebugPrint(llvm::BasicBlock* B, llvm::Value* TargetVal, std::unordered_map<llvm::Value*, llvm::Value*>& ValRe_mapper);
};

#endif// __ADD_DEBUG_PRINT