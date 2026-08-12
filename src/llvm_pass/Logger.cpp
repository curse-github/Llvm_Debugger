#define _BUILD_LOGGER

#include "Logger.h"
#include <iostream>
#include <algorithm>

llvm::FunctionType* logFunction_T;
llvm::Function* logFunctionParameters;
llvm::Function* logFunctionReturn;
llvm::PreservedAnalyses Logger::run(llvm::Module& Module, llvm::ModuleAnalysisManager& MAM) {
    populateGlobals(Module);
    logFunction_T = llvm::FunctionType::get(void_t, {ptr_t, ptr_t}, false);
    logFunctionParameters = llvm::Function::Create(logFunction_T, llvm::Function::LinkageTypes::ExternalLinkage, "logFunctionParameters", &Module);
    logFunctionReturn = llvm::Function::Create(logFunction_T, llvm::Function::LinkageTypes::ExternalLinkage, "logFunctionReturn", &Module);
    for(llvm::Function& F : Module) {
        std::string name = F.getName().str();
        if (!name.starts_with("llvm.") && !name.ends_with("_wrapper") && (name != "logFunctionParameters") && (name != "logFunctionReturn"))
            run(&F);
    }
    return llvm::PreservedAnalyses::none();
}
void Logger::run(llvm::Function* F) {
    std::string f_name = llvm::demangle(F->getName().str());
    int tmp1 = (int)f_name.size();
    if ((tmp1 = f_name.find('(')) != std::string::npos)
        f_name = f_name.substr(0, tmp1);
    for (llvm::User* tmp : F->users()) {
        llvm::CallInst* Inst = llvm::dyn_cast_or_null<llvm::CallInst>(tmp);
        if (Inst != nullptr) {
            if (!Inst->getParent()->getParent()->getName().str().ends_with("_wrapper")) {
                const unsigned int arg_size = F->arg_size();
                // get amount of storage needed
                unsigned int parameterBitWidth = 0;
                for (int i = 0; i < arg_size; i++)
                    parameterBitWidth += std::max(8, getTypeBitWidth(F->getArg(i)->getType()))>>3;
                // create variable
                llvm::ArrayType* T = llvm::ArrayType::get(i8_t, parameterBitWidth);
                llvm::Value* input = new llvm::AllocaInst(T, 0, "input", Inst);
                // populate buffer with data
                unsigned int runningOffset = 0;
                for(int i = 0; i < arg_size; i++) {
                    llvm::Instruction* func_argi_p = llvm::GetElementPtrInst::CreateInBounds(i8_t, input, { llvm::ConstantInt::get(i64_t, runningOffset) }, F->getName()+"_arg" + std::to_string(i) + "_p", Inst);
                    new llvm::StoreInst(Inst->getArgOperand(i), func_argi_p, Inst);
                    runningOffset += std::max(8, getTypeBitWidth(F->getArg(i)->getType()))>>3;
                }
                // log data
                llvm::CallInst::Create(logFunction_T, logFunctionParameters, { createGlobalString(F->getName().str()), input }, "", Inst);
                llvm::Type* retType = F->getReturnType();
                llvm::Instruction* afterInst = Inst->getNextNode();
                if (!retType->isVoidTy()) {
                    llvm::Value* output = new llvm::AllocaInst(retType, 0, "output", afterInst);
                    new llvm::StoreInst(Inst, output, afterInst);
                    llvm::CallInst::Create(logFunction_T, logFunctionReturn, { createGlobalString(F->getName().str()), output }, "", afterInst);
                } else
                    llvm::CallInst::Create(logFunction_T, logFunctionReturn, {
                        createGlobalString(F->getName().str()), llvm::ConstantPointerNull::get(llvm::dyn_cast<llvm::PointerType>(ptr_t))
                    }, "", afterInst);
            }
        }
    }
}