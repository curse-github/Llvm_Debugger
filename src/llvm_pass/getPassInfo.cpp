#include "Librarify.h"
#include "Logger.h"

llvm::PassPluginLibraryInfo getPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION, "LLVMpass", LLVM_VERSION_STRING, 
        [](llvm::PassBuilder& passBuilder) {
            passBuilder.registerPipelineParsingCallback(
                [](
                    llvm::StringRef Name, llvm::ModulePassManager& MPM, 
                    llvm::ArrayRef<llvm::PassBuilder::PipelineElement>
                ) {
                    if (Name == "librarify") {
                        MPM.addPass(Librarify());
                        return true;
                    } else if (Name == "logger") {
                        MPM.addPass(Logger());
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
    return getPluginInfo();
}
