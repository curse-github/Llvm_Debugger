#define _BUILD_DUP_B
#include "../include/AddDebugPrint.h"

#include <iostream>

llvm::PreservedAnalyses AddDebugPrint::run(llvm::Function& F, llvm::FunctionAnalysisManager& FAM) {
    std::cout << "Recevied function: \"" << F.getName().str() << "\"\n";
    return llvm::PreservedAnalyses::all();// llvm::PreservedAnalyses::none();
}


llvm::PassPluginLibraryInfo getAddDebugPrintPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION, "AddDebugPrint", LLVM_VERSION_STRING,
        [](llvm::PassBuilder& passBuilder) {
            passBuilder.registerPipelineParsingCallback(
                [](
                    llvm::StringRef Name, llvm::FunctionPassManager& FPM,
                    llvm::ArrayRef<llvm::PassBuilder::PipelineElement>
                ) {
                    if (Name == "add-debug-print") {
                        FPM.addPass(AddDebugPrint());
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
    return getAddDebugPrintPluginInfo();
}
