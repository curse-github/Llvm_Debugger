#define _BUILD_DUP_B
#include "../include/AddDebugPrint.h"

//#include <iostream>

llvm::PreservedAnalyses AddDebugPrint::run(llvm::Function& F, llvm::FunctionAnalysisManager& FAM) {
    // get function data
    llvm::Module* module = F.getParent();
    llvm::LLVMContext& context = module->getContext();
    llvm::BasicBlock* entry = &F.getEntryBlock();
    // create string to be used
    std::string str0_val = "calling " + F.getName().str() + "()\n";
    llvm::Type* char_t = llvm::Type::getInt8Ty(context);
    llvm::GlobalVariable* str0 = new llvm::GlobalVariable(*module, llvm::ArrayType::get(char_t, static_cast<unsigned int>(str0_val.size())+1u), true, llvm::GlobalValue::PrivateLinkage, 0, "str0");
    str0->setInitializer(llvm::ConstantDataArray::getString(context, str0_val, true));
    // create print function call
    llvm::Function* printStr = F.getParent()->getFunction("printStr");
    llvm::CallInst* debugPrint = llvm::CallInst::Create(printStr, { str0 }, "");
    debugPrint->setTailCall();
    // insert funtion call into function
    debugPrint->insertInto(entry, entry->begin());

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
