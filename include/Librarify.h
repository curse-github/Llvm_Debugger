#ifndef __LIBRARIFY
#define __LIBRARIFY
#ifdef _WIN32
    #ifdef _BUILD_LIBRARIFY
        #define LIBRARIFY_API __declspec(dllexport)
    #else
        #define LIBRARIFY_API __declspec(dllimport)
    #endif
#else
    #define LIBRARIFY_API
#endif

#include "llvmHelpers.h"

struct LIBRARIFY_API Librarify : public llvm::PassInfoMixin<Librarify> {
    static bool isRequired() { return true; }

    llvm::PreservedAnalyses run(llvm::Function& F, llvm::FunctionAnalysisManager&);
    void librarify(llvm::BasicBlock* B, llvm::Value* TargetVal, std::unordered_map<llvm::Value*, llvm::Value*>& ValRe_mapper);
};

#endif// __LIBRARIFY