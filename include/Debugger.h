#ifndef __DEBUGGER
#define __DEBUGGER
#ifdef _WIN32
    #ifdef _BUILD_DEBUGGER
        #define DEBUGGER_API __declspec(dllexport)
    #else
        #define DEBUGGER_API __declspec(dllimport)
    #endif
#else
    #define DEBUGGER_API
#endif

#include "llvmHelpers.h"

struct DEBUGGER_API Debugger : public llvm::PassInfoMixin<Debugger> {
    static bool isRequired() { return true; }

    llvm::PreservedAnalyses run(llvm::Module& Module, llvm::ModuleAnalysisManager& MAM);
    void run(llvm::Function* F);
    void debugger(llvm::BasicBlock* B, llvm::Value* TargetVal, std::unordered_map<llvm::Value*, llvm::Value*>& ValRe_mapper);
};

#endif// __DEBUGGER