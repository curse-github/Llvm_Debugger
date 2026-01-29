#define _BUILD_DUP_B
#include "../include/AddDebugPrint.h"

//#include <iostream>

llvm::Module* Module = nullptr;
llvm::LLVMContext* Context = nullptr;

llvm::Type* ptr_t = nullptr;
llvm::Type* char_t = nullptr;

llvm::CallInst* doCall(llvm::Function* f, std::string str, llvm::BasicBlock::iterator beforeInst) {
    // create string to be used
    llvm::GlobalVariable* gStr = new llvm::GlobalVariable(*Module, llvm::ArrayType::get(char_t, static_cast<unsigned int>(str.size())+1u), true, llvm::GlobalValue::PrivateLinkage, 0, "str");
    gStr->setInitializer(llvm::ConstantDataArray::getString(*Context, str, true));
    // create print function call
    llvm::CallInst* printCall = llvm::CallInst::Create(f, { gStr }, "");
    printCall->setTailCall();
    printCall->insertBefore(beforeInst);
    return printCall;
}
llvm::CallInst* doCall(llvm::Function* f, char chr, llvm::BasicBlock::iterator beforeInst) {
    // create string to be used
    llvm::GlobalVariable* gChr = new llvm::GlobalVariable(*Module, char_t, true, llvm::GlobalValue::PrivateLinkage, 0, "chr");
    gChr->setInitializer(llvm::ConstantInt::get(char_t, chr, true));
    // create print function call
    llvm::Instruction* loadChr = (new llvm::LoadInst(char_t, gChr, "chr", beforeInst));
    llvm::CallInst* printCall = llvm::CallInst::Create(f, { loadChr }, "");
    printCall->setTailCall();
    printCall->insertBefore(beforeInst);
    return printCall;
}

llvm::Function* printStr = nullptr;
llvm::Function* printChar = nullptr;
llvm::Function* printlnChar = nullptr;

llvm::PreservedAnalyses AddDebugPrint::run(llvm::Function& F, llvm::FunctionAnalysisManager& FAM) {
    // get function data
    if (!Module) {
        Module = F.getParent();
        Context = &Module->getContext();
        ptr_t = llvm::PointerType::get(*Context, 0);
        char_t = llvm::Type::getInt8Ty(*Context);

        printStr = F.getParent()->getFunction("printStr");
        printChar = F.getParent()->getFunction("printChar");
        printlnChar = F.getParent()->getFunction("printlnChar");
    }

    llvm::BasicBlock* eBlock = &F.getEntryBlock();
    //for(auto& arg : F.args())
        //std::cout << arg.getName().str() << '\n';
    llvm::BasicBlock::iterator beginning = eBlock->begin();
    doCall(printStr, F.getName().str(), beginning);
    doCall(printChar, '(', beginning);
    doCall(printlnChar, ')', beginning);
    // return
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
