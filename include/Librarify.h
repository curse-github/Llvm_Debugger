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
#include "llvm/Demangle/Demangle.h"
#include <vector>

struct LIBRARIFY_API Librarify : public llvm::PassInfoMixin<Librarify> {
    static bool isRequired() { return true; }

    llvm::PreservedAnalyses run(llvm::Module& Module, llvm::ModuleAnalysisManager& MAM);
    void run(llvm::Function& F, int tmp);
    void librarify(llvm::BasicBlock* B, llvm::Value* TargetVal, std::unordered_map<llvm::Value*, llvm::Value*>& ValRe_mapper);
};

#endif// __LIBRARIFY