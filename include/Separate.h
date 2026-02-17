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

#include "llvmHelpers.h"

struct SEPARATE_API Separate : public llvm::PassInfoMixin<Separate> {
    static bool isRequired() { return true; }

    llvm::PreservedAnalyses run(llvm::Function& F, llvm::FunctionAnalysisManager&);
    void separate(llvm::BasicBlock* B, llvm::Value* TargetVal, std::unordered_map<llvm::Value*, llvm::Value*>& ValRe_mapper);
};

#endif// __SEPARATE