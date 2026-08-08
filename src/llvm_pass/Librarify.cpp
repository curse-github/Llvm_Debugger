#define _BUILD_LIBRARIFY

#include "Librarify.h"
#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>

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
std::vector<llvm::Constant*> functionMangledNames_value;
std::vector<llvm::Constant*> functionNames_value;
std::vector<llvm::Constant*> functionReturnTypes_value;
std::vector<llvm::Constant*> functionParamCounts_value;
std::vector<llvm::Constant*> functionParamNames_value;
std::vector<llvm::Constant*> functionParamTypes_value;
std::vector<llvm::Constant*> functionPointers_value;

typedef struct {
    std::string returnType;
    std::vector<std::string> paramTypes;
    std::vector<std::string> paramNames;
} functionDataEntry;
std::map<std::string, functionDataEntry> functionData;

unsigned int numArgs = 0;
unsigned int numArgTypesDetermined = 0;
unsigned int numPointers = 0;
unsigned int numPointerTypesDetermined = 0;
llvm::PreservedAnalyses Librarify::run(llvm::Module& Module, llvm::ModuleAnalysisManager& MAM) {
    // read data from clang plugin (functionParams.csv)
    std::fstream* f = new std::fstream();
    f->open("tmp/functionParams.csv", std::ios::in);
    std::string lineString;
    while(std::getline(*f, lineString, '\n')) {
        functionDataEntry data;
        std::stringstream lineSStream(lineString);
        std::string returnType;
        if (!std::getline(lineSStream, returnType, ','))
            continue;
        data.returnType = returnType;
        std::string funcName;
        if (!std::getline(lineSStream, funcName, ','))
            continue;
        std::string numArgsString;
        if (!std::getline(lineSStream, numArgsString, ','))
            continue;
        unsigned int numArgs = std::stoi(numArgsString);
        // std::cout << returnType << ", " << funcName << ", " << numArgs;
        for (int i = 0; i < numArgs; i++) {
            std::string type;
            std::string name;
            if (!std::getline(lineSStream, type, ','))
                continue;
            if (!std::getline(lineSStream, name, ','))
                continue;
            data.paramTypes.push_back(type);
            data.paramNames.push_back(name);
            // std::cout << ", " << type << ' ' << name;
        }
        functionData[funcName] = data;
        // std::cout << '\n';
    }
    f->close();
    delete f;
    // read known struct types
    populateGlobals(Module);
    for (llvm::StructType* ST: Module.getIdentifiedStructTypes()) {
        structTypeToNameIndex[(llvm::Type*)ST] = namedStructCount;
        std::string structName = ST->getName().str();
        namedStructCount++;
        structTypes.push_back(ST);
        structNames.push_back(structName);
    }
    // loop through functions in module to create data lists
    for(llvm::Function& F : Module)
        if (!F.isDeclarationForLinker() && !F.getName().str().ends_with("_wrapper"))
            librarifyPass(F);
    //printStructTypes(Module);
    // std::cout << numPointerTypesDetermined << " out of " << numPointers << " (" << (numPointerTypesDetermined*100.0/std::max(1u, numPointers)) << "%) pointer types found\n";
    // std::cout << numArgTypesDetermined << " out of " << numArgs << " (" << (numArgTypesDetermined*100.0/numArgs) << "%) total types found\n";
    createGlobalInt(numFunctions_value, "numFunctions");
    createGlobalPtrArray(functionMangledNames_value, "functionMangledNames");
    createGlobalPtrArray(functionNames_value, "functionNames");
    createGlobalPtrArray(functionReturnTypes_value, "functionReturnTypes");
    createGlobalIntArray(functionParamCounts_value, "functionParamCounts");
    createGlobalPtrArray(functionParamNames_value, "functionParamNames");
    createGlobalPtrArray(functionParamTypes_value, "functionParamTypes");
    createGlobalPtrArray(functionPointers_value, "functionPointers");
    // create data lists for structs
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
    return llvm::PreservedAnalyses::none();
}
void Librarify::librarifyPass(llvm::Function& F) {
    // numFunctions
    numFunctions_value++;
    // functionMangledNames
    functionMangledNames_value.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalString(F.getName().str())));
    // functionNames
    std::string f_name = llvm::demangle(F.getName().str());
    int tmp1 = (int)f_name.size();
    if ((tmp1 = f_name.find('(')) != std::string::npos)
        f_name = f_name.substr(0, tmp1);
    if (f_name == "main")
        F.setName("old_"+F.getName().str());
    functionNames_value.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalString(f_name)));
    // check functionData
    bool foundFunctionDataMatch = false;
    if (functionData.count(f_name) > 0) {
        functionDataEntry& data = functionData[f_name];
        const unsigned int argSize = F.arg_size();
        if (argSize == data.paramNames.size()) {
            std::string llvmReturnType = basicGetTypeAsString(F.getReturnType());
            std::string clangReturnType = data.returnType;
            if ((llvmReturnType == clangReturnType) || ((llvmReturnType == "void*") && (clangReturnType[clangReturnType.size()-1] == '*'))) {
                foundFunctionDataMatch = true;
                for(unsigned int i = 0; (i < argSize)&&foundFunctionDataMatch; i++) {
                    std::string llvmArgName = F.getArg(i)->getName().str();
                    std::string clangArgName = data.paramNames[i];
                    if (llvmArgName != clangArgName)
                        foundFunctionDataMatch = false;
                    if (foundFunctionDataMatch) {
                        std::string llvmArgType = basicGetTypeAsString(F.getArg(i)->getType());
                        std::string clangArgType = data.paramTypes[i];
                        if (!(llvmArgType == clangArgType) && !((llvmArgType == "void*") && (clangArgType[clangArgType.size()-1] == '*'))) {
                            foundFunctionDataMatch = false;
                        }
                    }
                }
            }
        }
    }
    if (foundFunctionDataMatch) {
        functionDataEntry& data = functionData[f_name];
        // functionReturnTypes
        functionReturnTypes_value.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalString(data.returnType)));
        // functionParamCounts
        const unsigned int argSize = data.paramNames.size();
        functionParamCounts_value.push_back(llvm::ConstantInt::get(i32_t, argSize));
        // functionParamNames
        std::vector<llvm::Constant*> tmp_paramName_values;
        for(unsigned int i = 0; i < argSize; i++)
            tmp_paramName_values.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalString(data.paramNames[i])));
        functionParamNames_value.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalPtrArray(tmp_paramName_values, f_name + "_paramNames")));
        // functionParamTypes
        std::vector<llvm::Constant*> tmp_paramType_values;
        for(unsigned int i = 0; i < argSize; i++)
            tmp_paramType_values.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalString(data.paramTypes[i])));
        functionParamTypes_value.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalPtrArray(tmp_paramType_values, f_name + "_paramTypes")));
    } else {
        // functionReturnTypes
        functionReturnTypes_value.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalString(basicGetTypeAsString(F.getReturnType()))));
        // functionParamCounts
        const unsigned int argSize = F.arg_size();
        functionParamCounts_value.push_back(llvm::ConstantInt::get(i32_t, argSize));
        // functionParamNames
        std::vector<llvm::Constant*> tmp_paramName_values;
        for(unsigned int i = 0; i < argSize; i++)
            tmp_paramName_values.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalString(F.getArg(i)->getName().str())));
        functionParamNames_value.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalPtrArray(tmp_paramName_values, f_name + "_paramNames")));
        // functionParamTypes
        std::vector<llvm::Constant*> tmp_paramType_values;
        for(unsigned int i = 0; i < argSize; i++) {
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
        functionParamTypes_value.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalPtrArray(tmp_paramType_values, f_name + "_paramTypes")));
    }
    // functionPointers
    llvm::FunctionType* wrapper_f_t = llvm::FunctionType::get(F.getReturnType(), { ptr_t }, false);
    llvm::Function* wrapper_f = llvm::Function::Create(wrapper_f_t, llvm::Function::LinkageTypes::InternalLinkage, F.getName().str()+"_wrapper", Module);
    llvm::Argument* buffer = wrapper_f->getArg(0);
    buffer->setName("buffer");
    llvm::BasicBlock* wrapper_entry = llvm::BasicBlock::Create(*Context, "", wrapper_f);
    std::vector<llvm::Value*> args;
    unsigned long long int offset = 0;
    const unsigned int argSize = F.arg_size();
    for(unsigned int i = 0; i < argSize; i++) {
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