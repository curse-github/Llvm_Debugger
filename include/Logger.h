#ifndef __LOGGER
#define __LOGGER
#ifdef _WIN32
    #ifdef _BUILD_LOGGER
        #define LOGGER_API __declspec(dllexport)
    #else
        #define LOGGER_API __declspec(dllimport)
    #endif
#else
    #define LOGGER_API
#endif

#include "llvmHelpers.h"

struct LOGGER_API Logger : public llvm::PassInfoMixin<Logger> {
    static bool isRequired() { return true; }

    llvm::PreservedAnalyses run(llvm::Module& Module, llvm::ModuleAnalysisManager& MAM);
    void run(llvm::Function* F);
    void logger(llvm::BasicBlock* B, llvm::Value* TargetVal, std::unordered_map<llvm::Value*, llvm::Value*>& ValRe_mapper);
};

#endif// __LOGGER