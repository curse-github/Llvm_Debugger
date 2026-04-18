#define _BUILD_LIBRARIFY

#include "Librarify.h"
#include <iostream>

void printFuncSig(const llvm::Function& F) {
    std::cout << llvm::demangle(F.getName().str()) << "(";
    const unsigned int arg_size = F.arg_size();
    for(unsigned int i = 0; i < arg_size; i++) {
        llvm::Argument* arg = F.getArg(i);
        std::cout << ((i != 0) ? ", " : "") << getTypeAsString(arg);
    }
    std::cout << ")\n";
}

int numFunctions_value = 0;
std::vector<llvm::Constant*> functionNames_value;
std::vector<llvm::Constant*> functionParamCounts_value;
std::vector<llvm::Constant*> functionParamNames_value;
std::vector<llvm::Constant*> functionParamTypes_value;
std::vector<llvm::Constant*> functionPointers_value;

llvm::PreservedAnalyses Librarify::run(llvm::Module& Module, llvm::ModuleAnalysisManager& MAM) {
    populateGlobals(Module);
    int i = 0;
    for(llvm::Function& F : Module) {
        if (!F.isDeclarationForLinker() && !F.getName().str().ends_with("_wrapper"))
            run(F, i);
        i++;
    }
    createGlobalInt(numFunctions_value, "numFunctions");
    createGlobalPtrArray(functionNames_value, "functionNames");
    createGlobalIntArray(functionParamCounts_value, "functionParamCounts");
    createGlobalPtrArray(functionParamNames_value, "functionParamNames");
    createGlobalPtrArray(functionParamTypes_value, "functionParamTypes");
    createGlobalPtrArray(functionPointers_value, "functionPointers");
    return llvm::PreservedAnalyses::none();
}
void Librarify::run(llvm::Function& F, int tmp) {
    //printFuncSig(F);
    // numFunctions
    numFunctions_value++;
    // functionNames
    const std::string f_name = llvm::demangle(F.getName().str());
    if (f_name == "main")
        F.setName("old_"+F.getName().str());
    functionNames_value.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalString(f_name)));
    // functionParamCounts
    const unsigned int arg_size = F.arg_size();
    std::cout << f_name << '(' << arg_size << ")\n";
    functionParamCounts_value.push_back(llvm::ConstantInt::get(i32_t, arg_size));
    // functionParamNames
    std::vector<llvm::Constant*> tmp_paramName_values;
    for(unsigned int i = 0; i < arg_size; i++)
        tmp_paramName_values.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalString(F.getArg(i)->getName().str())));
    functionParamNames_value.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalPtrArray(tmp_paramName_values, f_name + "_paramNames")));
    // functionParamTypes
    std::vector<llvm::Constant*> tmp_paramType_values;
    for(unsigned int i = 0; i < arg_size; i++)
        tmp_paramType_values.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalString(getTypeAsString(F.getArg(i)))));
    functionParamTypes_value.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalPtrArray(tmp_paramType_values, f_name + "_paramTypes")));
    // functionPointers
    llvm::FunctionType* wrapper_f_t = llvm::FunctionType::get(F.getReturnType(), { ptr_t }, false);
    llvm::Function* wrapper_f = llvm::Function::Create(wrapper_f_t, llvm::Function::LinkageTypes::InternalLinkage, f_name+"_wrapper", Module);
    llvm::Argument* buffer = wrapper_f->getArg(0);
    buffer->setName("buffer");
    llvm::BasicBlock* wrapper_entry = llvm::BasicBlock::Create(*Context, "", wrapper_f);
    std::vector<llvm::Value*> args;
    unsigned long long int offset = 0;
    for(unsigned int i = 0; i < arg_size; i++) {
        // load value as correct type
        const int bitWidth = getTypeBitWidth(F.getArg(i)->getType());
        const int size = std::max(8, bitWidth) >> 3;
        if (offset == 0) {
            llvm::Instruction* val = new llvm::LoadInst(F.getArg(i)->getType(), buffer, "arg_"+std::to_string(i), wrapper_entry);
            args.push_back(val);
        } else {
            llvm::Instruction* arg_i_p = llvm::GetElementPtrInst::CreateInBounds(i8_t, buffer, { llvm::ConstantInt::get(i64_t, offset) }, "arg" + std::to_string(i) + "_p", nullptr);
            arg_i_p->insertInto(wrapper_entry, wrapper_entry->end());
            llvm::Instruction* val = new llvm::LoadInst(F.getArg(i)->getType(), arg_i_p, "arg_"+std::to_string(i), wrapper_entry);
            args.push_back(val);
        }
        offset += size;
    }
    llvm::CallInst* out = llvm::CallInst::Create(&F, args, "");
    out->setTailCall();
    out->insertInto(wrapper_entry, wrapper_entry->end());
    if (F.getReturnType()->isVoidTy()) {
        llvm::ReturnInst::Create(*Context, wrapper_entry);
    } else {
        out->setName("out");
        llvm::ReturnInst::Create(*Context, out)->insertInto(wrapper_entry, wrapper_entry->end());
    }
    // functionPointers_value
    functionPointers_value.push_back(llvm::dyn_cast<llvm::Constant>(wrapper_f));
}


llvm::PassPluginLibraryInfo getLibrarifyPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION, "Librarify", LLVM_VERSION_STRING,
        [](llvm::PassBuilder& passBuilder) {
            passBuilder.registerPipelineParsingCallback(
                [](
                    llvm::StringRef Name, llvm::ModulePassManager& MPM,
                    llvm::ArrayRef<llvm::PassBuilder::PipelineElement>
                ) {
                    if (Name == "librarify") {
                        MPM.addPass(Librarify());
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
