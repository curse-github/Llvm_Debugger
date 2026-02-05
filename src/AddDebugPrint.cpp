#define _BUILD_DUP_B
#include "../include/AddDebugPrint.h"

//#include <iostream>

llvm::PreservedAnalyses AddDebugPrint::run(llvm::Function& F, llvm::FunctionAnalysisManager& FAM) {
    populateGlobals(F);
    llvm::BasicBlock* eBlock = &F.getEntryBlock();
    llvm::BasicBlock::iterator beginning = eBlock->begin();
    llvm::GlobalVariable* commaStr = createGlobalString(", ");
    doCall(printStr, createGlobalString(F.getName().str()), beginning);
    doCall(printChar, '(', beginning);
    //std::cout << "looking at " << F.getName().str() << '(';
    const unsigned int arg_size = F.arg_size();
    for(unsigned int i = 0; i < arg_size; i++) {
        llvm::Argument* arg = F.getArg(i);
        //std::cout << getTypeString(((llvm::Value*)arg)->getType()) << ' ' << arg->getName().str();
        tryPrintValue(arg, beginning);
        if (i != (arg_size-1)) {
            //std::cout << ", ";
            doCall(printStr, commaStr, beginning);
        }
        //doCall(printStr, arg, beginning);
    }
    //std::cout << ")\n";
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
