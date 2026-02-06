#define _BUILD_DUP_B
#include "AddDebugPrint.h"

llvm::GlobalVariable* calledStr = nullptr;
llvm::GlobalVariable* commaStr = nullptr;

llvm::PreservedAnalyses AddDebugPrint::run(llvm::Function& F, llvm::FunctionAnalysisManager& FAM) {
    populateGlobals(F);
    if (calledStr == nullptr) {
        calledStr = createGlobalString("called ");
        commaStr = createGlobalString(", ");
    }
    llvm::BasicBlock* eBlock = &F.getEntryBlock();
    llvm::BasicBlock::iterator beginning = eBlock->begin();
    doCall(printStr, calledStr, beginning);
    doCall(printStr, createGlobalString(F.getName().str()), beginning);
    doCall(printChar, '(', beginning);
    const unsigned int arg_size = F.arg_size();
    for(unsigned int i = 0; i < arg_size; i++) {
        llvm::Argument* arg = F.getArg(i);
        tryPrintValue(arg, beginning);
        if (i != (arg_size-1))
            doCall(printStr, commaStr, beginning);
    }
    doCall(printlnChar, ')', beginning);
    return llvm::PreservedAnalyses::none();
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
