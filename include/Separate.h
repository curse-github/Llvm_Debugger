#ifndef __SEPARATE
#define __SEPARATE
#ifdef _WIN32
    #ifdef _BUILD_SEPARATE
        #define SEPARATE_API __declspec(dllexport)
    #else
        #define SEPARATE_API __declspec(dllimport)
    #endif
#else
    #define SEPARATE_API
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
#include "llvm/Support/raw_ostream.h"
#include <llvm/TargetParser/Host.h>

struct SEPARATE_API Separate : public llvm::PassInfoMixin<Separate> {
    static bool isRequired() { return true; }

    llvm::PreservedAnalyses run(llvm::Function& F, llvm::FunctionAnalysisManager&);
    void separate(llvm::BasicBlock* B, llvm::Value* TargetVal, std::unordered_map<llvm::Value*, llvm::Value*>& ValRe_mapper);
};

#endif// __SEPARATE