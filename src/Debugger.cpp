#define _BUILD_DEBUGGER

#include "Debugger.h"
#include <iostream>
#include <algorithm>

void printStructTypes(llvm::Module& Module) {
    std::cout << "structs : {\n";
    for(unsigned int i = 0u; i < namedStructCount; i++) {
        llvm::StructType* ST = structTypes[i];
        std::cout << "    " << structNames[i] << " : {\n";
        for(int i = 0; i < ST->getNumElements(); i++)
            std::cout << "        " << basicGetTypeAsString(ST->getTypeAtIndex(i)) << '\n';
        std::cout << "    }\n";
    }
    std::cout << "}\n";
}

unsigned int numFunctions_value = 0;
std::vector<llvm::Constant*> functionNames_value;
std::vector<llvm::Constant*> functionReturnTypes_value;
std::vector<llvm::Constant*> functionParamCounts_value;
std::vector<llvm::Constant*> functionParamNames_value;
std::vector<llvm::Constant*> functionParamTypes_value;
std::vector<llvm::Constant*> functionPointers_value;

unsigned int numArgs = 0;
unsigned int numArgTypesDetermined = 0;
unsigned int numPointers = 0;
unsigned int numPointerTypesDetermined = 0;

llvm::FunctionType* logFunction_T;
llvm::Function* logFunctionParameters;
llvm::Function* logFunctionReturn;
llvm::PreservedAnalyses Debugger::run(llvm::Module& Module, llvm::ModuleAnalysisManager& MAM) {
    populateGlobals(Module);
    for (llvm::StructType* ST: Module.getIdentifiedStructTypes()) {
        structTypeToNameIndex[(llvm::Type*)ST] = namedStructCount;
        namedStructCount++;
        structTypes.push_back(ST);
        structNames.push_back(ST->getName().str());
    }
    for(llvm::Function& F : Module)
        if (!F.isDeclarationForLinker() && !F.getName().str().ends_with("_wrapper"))
            librarifyPass(F);
    //printStructTypes(Module);
    std::cout << numPointerTypesDetermined << " out of " << numPointers << " (" << (numPointerTypesDetermined*100.0/std::max(1u, numPointers)) << "%) pointer types found\n";
    std::cout << numArgTypesDetermined << " out of " << numArgs << " (" << (numArgTypesDetermined*100.0/numArgs) << "%) total types found\n";
    createGlobalInt(numFunctions_value, "numFunctions");
    createGlobalPtrArray(functionNames_value, "functionNames");
    createGlobalPtrArray(functionReturnTypes_value, "functionReturnTypes");
    createGlobalIntArray(functionParamCounts_value, "functionParamCounts");
    createGlobalPtrArray(functionParamNames_value, "functionParamNames");
    createGlobalPtrArray(functionParamTypes_value, "functionParamTypes");
    createGlobalPtrArray(functionPointers_value, "functionPointers");

    std::vector<llvm::Constant*> structNamesValues;
    std::vector<llvm::Constant*> structNumContainedTypesValues;
    std::vector<llvm::Constant*> structContainedTypesValues;
    for (int i = 0; i < namedStructCount; i++) {
        structNamesValues.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalString(structNames[i].c_str())));
        structNumContainedTypesValues.push_back(llvm::dyn_cast<llvm::Constant>(llvm::ConstantInt::get(i32_t, structTypes[i]->getNumElements())));
        std::vector<llvm::Constant*> tmp;
        for (int j = 0; j < structTypes[i]->getNumElements(); j++)
            tmp.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalString(basicGetTypeAsString(structTypes[i]->getTypeAtIndex(j)))));
        structContainedTypesValues.push_back(createGlobalPtrArray(tmp, structNames[i] + ".containedTypes"));
    }
    createGlobalInt(namedStructCount, "numStructs");
    createGlobalPtrArray(structNamesValues, "structNames");
    createGlobalIntArray(structNumContainedTypesValues, "structNumContainedTypes");
    createGlobalPtrArray(structContainedTypesValues, "structContainedTypes");


    logFunction_T = llvm::FunctionType::get(void_t, {ptr_t, ptr_t}, false);
    logFunctionParameters = llvm::Function::Create(logFunction_T, llvm::Function::LinkageTypes::ExternalLinkage, "logFunctionParameters", &Module);
    logFunctionReturn = llvm::Function::Create(logFunction_T, llvm::Function::LinkageTypes::ExternalLinkage, "logFunctionReturn", &Module);
    for(llvm::Function& F : Module)
        if (!F.isDeclarationForLinker() && !F.getName().str().ends_with("_wrapper"))
            debuggerPass(&F);
    return llvm::PreservedAnalyses::none();
}
void Debugger::librarifyPass(llvm::Function& F) {
    // numFunctions
    numFunctions_value++;
    // functionNames
    std::string f_name = llvm::demangle(F.getName().str());
    int tmp1 = (int)f_name.size();
    if ((tmp1 = f_name.find('(')) != std::string::npos)
        f_name = f_name.substr(0, tmp1);
    if (f_name == "main")
        F.setName("old_"+F.getName().str());
    functionNames_value.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalString(f_name)));
    // functionReturnTypes
    functionReturnTypes_value.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalString(basicGetTypeAsString(F.getReturnType()))));
    // functionParamCounts
    const unsigned int arg_size = F.arg_size();
    functionParamCounts_value.push_back(llvm::ConstantInt::get(i32_t, arg_size));
    // functionParamNames
    std::vector<llvm::Constant*> tmp_paramName_values;
    for(unsigned int i = 0; i < arg_size; i++)
        tmp_paramName_values.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalString(F.getArg(i)->getName().str())));
    functionParamNames_value.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalPtrArray(tmp_paramName_values, f_name + "_paramNames")));
    // functionParamTypes
    std::vector<llvm::Constant*> tmp_paramType_values;
    for(unsigned int i = 0; i < arg_size; i++) {
        //std::cout << f_name << '.' << F.getArg(i)->getName().str() << '\n';
        std::string tmp = getTypeAsString(F.getArg(i));
        numArgs++;
        if (tmp.ends_with('*')) {
            if (F.getArg(i)->hasByValAttr())
                tmp = basicGetTypeAsString(F.getParamByValType(i)) + '*';
            numPointers++;
            if (tmp != "void*") {
                numArgTypesDetermined++;
                numPointerTypesDetermined++;
            }
        } else
            numArgTypesDetermined++;
        tmp_paramType_values.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalString(tmp)));
    }
    // the char** sometimes cannot be determined, and therefor becomes void*, this fixes that
    if ((f_name == "main") && (arg_size == 2))
        tmp_paramType_values[1] = llvm::dyn_cast<llvm::Constant>(createGlobalString("char**"));
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
void Debugger::debuggerPass(llvm::Function* F) {
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
                llvm::CallInst::Create(logFunction_T, logFunctionParameters, { createGlobalString(f_name), input }, "", Inst);
                llvm::Type* retType = F->getReturnType();
                llvm::Instruction* afterInst = Inst->getNextNode();
                if (!retType->isVoidTy()) {
                    llvm::Value* output = new llvm::AllocaInst(retType, 0, "output", afterInst);
                    new llvm::StoreInst(Inst, output, afterInst);
                    llvm::CallInst::Create(logFunction_T, logFunctionReturn, { createGlobalString(f_name), output }, "", afterInst);
                } else
                    llvm::CallInst::Create(logFunction_T, logFunctionReturn, {
                        createGlobalString(f_name), llvm::ConstantPointerNull::get(llvm::dyn_cast<llvm::PointerType>(ptr_t))
                    }, "", afterInst);
            }
        }
    }
}


llvm::PassPluginLibraryInfo getDebuggerPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION, "Debugger", LLVM_VERSION_STRING, 
        [](llvm::PassBuilder& passBuilder) {
            passBuilder.registerPipelineParsingCallback(
                [](
                    llvm::StringRef Name, llvm::ModulePassManager& MPM, 
                    llvm::ArrayRef<llvm::PassBuilder::PipelineElement>
                ) {
                    if (Name == "debugger") {
                        MPM.addPass(Debugger());
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
    return getDebuggerPluginInfo();
}
