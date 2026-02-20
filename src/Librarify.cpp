#define _BUILD_LIBRARIFY

#include "Librarify.h"
#include <iostream>

llvm::PreservedAnalyses Librarify::run(llvm::Function& F, llvm::FunctionAnalysisManager& FAM) {
    
    return llvm::PreservedAnalyses::all();
}


llvm::PassPluginLibraryInfo getLibrarifyPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION, "Librarify", LLVM_VERSION_STRING,
        [](llvm::PassBuilder& passBuilder) {
            passBuilder.registerPipelineParsingCallback(
                [](
                    llvm::StringRef Name, llvm::FunctionPassManager& FPM,
                    llvm::ArrayRef<llvm::PassBuilder::PipelineElement>
                ) {
                    if (Name == "librarify") {
                        FPM.addPass(Librarify());
                        return true;
                    }
                    return false;
                }
            );
        }
    };
}
#ifdef _WIN32
    #pragma comment(linker, "/EXPORT:llvmGetPassPluginInfo")
#endif
extern "C"
llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return getLibrarifyPluginInfo();
}
