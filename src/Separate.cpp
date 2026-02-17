#define _BUILD_SEPARATE

#include "Separate.h"
#include <iostream>

void printFuncSig(llvm::Function& F) {
    std::cout << F.getName().str() << "(";
    const unsigned int arg_size = F.arg_size();
    for(unsigned int i = 0; i < arg_size; i++) {
        llvm::Argument* arg = F.getArg(i);
        std::cout << ((i != 0) ? ", " : "") << getTypeAsString(arg);
    }
    std::cout << ")\n";
}

llvm::PreservedAnalyses Separate::run(llvm::Function& F, llvm::FunctionAnalysisManager& FAM) {
    llvm::LLVMContext Context;
    llvm::Type* ptr_t = llvm::PointerType::get(Context, 0);
    llvm::Type* i32_t = llvm::Type::getInt32Ty(Context);
    llvm::Module* Module = new llvm::Module(F.getName().str(), Context);
    Module->setTargetTriple(llvm::Triple(llvm::sys::getDefaultTargetTriple()));

    llvm::FunctionType* main_t = llvm::FunctionType::get(i32_t, { i32_t, ptr_t }, false);
    llvm::Function* main = llvm::Function::Create(main_t, llvm::Function::ExternalLinkage, "main", Module);
    llvm::Argument* main_argc = main->getArg(0);
    main_argc->setName("argc");
    llvm::Argument* main_argv = main->getArg(1);
    main_argv->setName("argv");
    llvm::BasicBlock* mainRetBlock = llvm::BasicBlock::Create(Context, "", main);
    llvm::ReturnInst::Create(Context, llvm::ConstantInt::get(i32_t, 0))->insertInto(mainRetBlock, mainRetBlock->end());

    for(llvm::BasicBlock& B : F) {
        for(llvm::Instruction& I : B) {
            llvm::CallInst* inst = llvm::dyn_cast_or_null<llvm::CallInst>(&I);
            if (inst == nullptr) continue;
            llvm::Function* calledFunc = inst->getCalledFunction();
            if (calledFunc->isDeclaration()) continue;
            printFuncSig(*calledFunc);
            llvm::Function::Create(calledFunc->getFunctionType(), llvm::Function::InternalLinkage, calledFunc->getName().str(), Module);
        }
    }

    std::string newFName = F.getName().str();
    if (newFName == "main") newFName += "_2";
    llvm::Function* newF = llvm::Function::Create(F.getFunctionType(), llvm::Function::InternalLinkage, newFName, Module);
    llvm::ValueToValueMapTy map;
    llvm::SmallVector<llvm::ReturnInst*> returns;
    llvm::CloneFunctionBodyInto(*newF, F, map, llvm::RemapFlags::RF_None, returns);

    std::error_code EC;
    llvm::raw_fd_ostream outFile(std::string("./separated/") + F.getName().str() + ".ll", EC);
    Module->print(outFile, nullptr);
    delete Module;
    return llvm::PreservedAnalyses::all();
}


llvm::PassPluginLibraryInfo getSeparatePluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION, "Separate", LLVM_VERSION_STRING,
        [](llvm::PassBuilder& passBuilder) {
            passBuilder.registerPipelineParsingCallback(
                [](
                    llvm::StringRef Name, llvm::FunctionPassManager& FPM,
                    llvm::ArrayRef<llvm::PassBuilder::PipelineElement>
                ) {
                    if (Name == "separate") {
                        FPM.addPass(Separate());
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
    return getSeparatePluginInfo();
}
