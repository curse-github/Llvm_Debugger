#define _BUILD_LIBRARIFY

#include "Librarify.h"
#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>

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
std::map<std::string, std::string> enumTypesMap;

unsigned int numArgs = 0;
unsigned int numArgTypesDetermined = 0;
unsigned int numPointers = 0;
unsigned int numPointerTypesDetermined = 0;
void readFunctionParamsCsv() {
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
            if (!std::getline(lineSStream, type, ','))
                continue;
            std::string name;
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
}
std::vector<llvm::Constant*> structNamesValues;
std::vector<llvm::Constant*> structNumFieldsValues;
std::vector<llvm::Constant*> structFieldTypesValues;
std::vector<llvm::Constant*> structFieldNamesValues;
void readTypeDefsCsv() {
    unsigned int numEnums = 0;
    std::vector<llvm::Constant*> enumNames_value;
    std::vector<llvm::Constant*> enumTypes_value;
    std::vector<llvm::Constant*> enumNumValues_value;
    std::vector<llvm::Constant*> enumValueNames_value;
    std::vector<llvm::Constant*> enumValueValues_value;
    std::fstream* TypedefsIn = new std::fstream();
    TypedefsIn->open("tmp/typedefs.csv", std::ios::in);
    std::string lineString;
    while(std::getline(*TypedefsIn, lineString, '\n')) {
        std::stringstream lineSStream(lineString);
        std::string type;
        if (!std::getline(lineSStream, type, ','))
            continue;
        if (type == "enum") {
            std::string enumIntType;
            if (!std::getline(lineSStream, type, ','))
                continue;
            enumTypes_value.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalString(enumIntType)));
            std::string enumName;
            if (!std::getline(lineSStream, enumName, ','))
                continue;
            enumName = "enum."+enumName;
            enumTypesMap[enumName] = enumIntType;
            enumNames_value.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalString(enumName)));
            std::string numValuesString;
            if (!std::getline(lineSStream, numValuesString, ','))
                continue;
            unsigned int numValues = std::stoi(numValuesString);
            enumNumValues_value.push_back(llvm::ConstantInt::get(i32_t, numValues));
            std::vector<llvm::Constant*> tmp_valueNames;
            std::vector<llvm::Constant*> tmp_valueValues;
            for (int i = 0; i < numValues; i++) {
                std::string valueString;
                if (!std::getline(lineSStream, valueString, ','))
                    continue;
                int value = std::stoi(valueString);
                tmp_valueValues.push_back(llvm::ConstantInt::get(i32_t, value));
                std::string name;
                if (!std::getline(lineSStream, name, ','))
                    continue;
                tmp_valueNames.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalString(name)));
            }
            enumValueNames_value.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalPtrArray(tmp_valueNames, enumName + "_valueNames")));
            enumValueValues_value.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalIntArray(tmp_valueValues, enumName + "_valueValues")));
            numEnums++;
        } else if (type == "struct") {
            std::string structName;
            if (!std::getline(lineSStream, structName, ','))
                continue;
            structName = "struct."+structName;
            structNameToIndex[structName] = structCount;
            structCount++;
            structNames.push_back(structName);

            structNamesValues.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalString(structName.c_str())));
            std::string numFieldsString;
            if (!std::getline(lineSStream, numFieldsString, ','))
                continue;
            unsigned int numFields = std::stoi(numFieldsString);
            structNumFieldsValues.push_back(llvm::dyn_cast<llvm::Constant>(llvm::ConstantInt::get(i32_t, numFields)));
            std::vector<llvm::Constant*> tmp_FieldTypes;
            std::vector<llvm::Constant*> tmp_FieldNames;
            for (int j = 0; j < numFields; j++) {
                std::string fieldType;
                if (!std::getline(lineSStream, fieldType, ','))
                    continue;
                std::string fieldName;
                if (!std::getline(lineSStream, fieldName, ','))
                    continue;
                tmp_FieldTypes.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalString(fieldType)));
                tmp_FieldNames.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalString(fieldName)));
            }
            structFieldTypesValues.push_back(createGlobalPtrArray(tmp_FieldTypes, structName + ".fieldTypes"));
            structFieldNamesValues.push_back(createGlobalPtrArray(tmp_FieldNames, structName + ".fieldNames"));
        } else
            std::cout << "ERROR!\n";
    }
    TypedefsIn->close();
    delete TypedefsIn;
    createGlobalInt(numEnums, "numEnums");
    createGlobalPtrArray(enumNames_value, "enumNames");
    createGlobalPtrArray(enumTypes_value, "enumTypes");
    createGlobalIntArray(enumNumValues_value, "enumNumValues");
    createGlobalPtrArray(enumValueNames_value, "enumValueNames");
    createGlobalPtrArray(enumValueValues_value, "enumValueValues");
}
llvm::PreservedAnalyses Librarify::run(llvm::Module& Module, llvm::ModuleAnalysisManager& MAM) {
    populateGlobals(Module);
    // read data from csv files created by clang plugin
    readTypeDefsCsv();
    readFunctionParamsCsv();
    // read known struct types
    for (llvm::StructType* ST: Module.getIdentifiedStructTypes()) {
        std::string structName = ST->getName().str();
        if (structNameToIndex.count(structName) > 0) {
            structTypeToIndex[(llvm::Type*)ST] = structNameToIndex[structName];
        } else {
            structTypeToIndex[(llvm::Type*)ST] = structCount;
            structNameToIndex[structName] = structCount;
            structCount++;
            structNames.push_back(structName);

            structNamesValues.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalString(structName.c_str())));
            structNumFieldsValues.push_back(llvm::dyn_cast<llvm::Constant>(llvm::ConstantInt::get(i32_t, ST->getNumElements())));
            std::vector<llvm::Constant*> tmp_FieldTypes;
            for (int j = 0; j < ST->getNumElements(); j++)
                tmp_FieldTypes.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalString(basicGetTypeAsString(ST->getTypeAtIndex(j)))));
            structFieldTypesValues.push_back(createGlobalPtrArray(tmp_FieldTypes, structName + ".fieldTypes"));
            std::vector<llvm::Constant*> tmp_FieldNames;
            for (int j = 0; j < ST->getNumElements(); j++)
                tmp_FieldNames.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalString(std::to_string(j))));
            structFieldNamesValues.push_back(createGlobalPtrArray(tmp_FieldNames, structName + ".fieldNames"));
        }
    }
    // loop through functions in module to create data lists
    for(llvm::Function& F : Module) {
        std::string name = F.getName().str();
        if (!name.starts_with("llvm.") && !name.ends_with("_wrapper") && !F.isVarArg())
            run(F);
    }
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
    for (int i = 0; i < unnamedStructTypes.size(); i++) {
        llvm::StructType *ST = unnamedStructTypes[i];
        std::string name = "struct.unknown" + std::to_string(i);
        structNamesValues.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalString(name.c_str())));
        structNumFieldsValues.push_back(llvm::dyn_cast<llvm::Constant>(llvm::ConstantInt::get(i32_t, ST->getNumElements())));
        std::vector<llvm::Constant*> tmp_FieldTypes;
        for (int j = 0; j < ST->getNumElements(); j++)
            tmp_FieldTypes.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalString(basicGetTypeAsString(ST->getTypeAtIndex(j)))));
        structFieldTypesValues.push_back(createGlobalPtrArray(tmp_FieldTypes, name + ".fieldTypes"));
        std::vector<llvm::Constant*> tmp_FieldNames;
        for (int j = 0; j < ST->getNumElements(); j++)
            tmp_FieldNames.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalString(std::to_string(j))));
        structFieldNamesValues.push_back(createGlobalPtrArray(tmp_FieldNames, name + ".fieldNames"));
    }
    createGlobalInt(structCount, "numStructs");
    createGlobalPtrArray(structNamesValues, "structNames");
    createGlobalIntArray(structNumFieldsValues, "structNumFields");
    createGlobalPtrArray(structFieldTypesValues, "structFieldTypes");
    createGlobalPtrArray(structFieldNamesValues, "structFieldNames");
    return llvm::PreservedAnalyses::none();
}
void Librarify::run(llvm::Function& F) {
    std::string f_name = llvm::demangle(F.getName().str());
    // numFunctions
    numFunctions_value++;
    // functionMangledNames
    functionMangledNames_value.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalString(F.getName().str())));
    // functionNames
    int tmp1 = (int)f_name.size();
    if ((tmp1 = f_name.find('(')) != std::string::npos)
        f_name = f_name.substr(0, tmp1);
    if (f_name == "main")
        F.setName("old_"+F.getName().str());
    functionNames_value.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalString(f_name)));
    // check functionData
    bool foundFunctionDataMatch = false;
    if (functionData.count(f_name) > 0) {
        std::string reason = "";
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
                    std::string llvmArgType = basicGetTypeAsString(F.getArg(i)->getType());
                    std::string clangArgType = data.paramTypes[i];
                    if (!(llvmArgType == clangArgType) && !((llvmArgType == "void*") && (clangArgType[clangArgType.size()-1] == '*'))) {
                        foundFunctionDataMatch = false;
                        for(auto const& p : enumTypesMap) {
                            size_t tmp = clangArgType.find(p.first);
                            if (tmp != std::string::npos) {
                                if (clangArgType.replace(tmp,p.first.size(),p.second) == llvmArgType) {
                                    foundFunctionDataMatch = true;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
        if (!foundFunctionDataMatch) {
            std::cout << "failed to find match for function \"" << f_name << "\" for reason \"" << reason << "\"\n";
        }
    }
    if (foundFunctionDataMatch) {
        //std::cout << "Found match!\n";
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