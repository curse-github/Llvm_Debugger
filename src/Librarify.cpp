#define _BUILD_LIBRARIFY

#include "Librarify.h"
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

int numFunctions_value = 0;
std::vector<llvm::Constant*> functionNames_value;
std::vector<llvm::Constant*> functionParamCounts_value;
std::vector<llvm::Constant*> functionParamNames_value;
std::vector<llvm::Constant*> functionParamTypes_value;

llvm::PreservedAnalyses Librarify::run(llvm::Function& F, llvm::FunctionAnalysisManager& FAM) {
    populateGlobals(F);
    // numFunctions
    numFunctions_value++;
    // functionNames
    functionNames_value.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalString(F.getName().str())));
    // functionParamCounts
    const unsigned int arg_size = F.arg_size();
    functionParamCounts_value.push_back(llvm::ConstantInt::get(i32_t, arg_size));
    // functionParamNames
    std::vector<llvm::Constant*> tmp_paramName_values;
    for(unsigned int i = 0; i < arg_size; i++)
        tmp_paramName_values.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalString(F.getArg(i)->getName().str())));
    functionParamNames_value.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalPtrArray(tmp_paramName_values, F.getName().str() + "_paramNames")));
    // functionParamTypes
    std::vector<llvm::Constant*> tmp_paramType_values;
    for(unsigned int i = 0; i < arg_size; i++)
        tmp_paramType_values.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalString(getTypeAsString(F.getArg(i)))));
    functionParamTypes_value.push_back(llvm::dyn_cast<llvm::Constant>(createGlobalPtrArray(tmp_paramType_values, F.getName().str() + "_paramTypes")));
    // functionPointers


    createGlobalInt(numFunctions_value, "numFunctions");
    createGlobalPtrArray(functionNames_value, "functionNames");
    createGlobalIntArray(functionParamCounts_value, "functionParamCounts");
    createGlobalPtrArray(functionParamNames_value, "functionParamNames");
    createGlobalPtrArray(functionParamTypes_value, "functionParamTypes");
    return llvm::PreservedAnalyses::none();
}


llvm::PassPluginLibraryInfo getLibrarifyPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION, "Librarify", LLVM_VERSION_STRING,
        [](llvm::PassBuilder& passBuilder) {
            passBuilder.registerPipelineParsingCallback(
                [](
                    llvm::StringRef Name, llvm::FunctionPassManager& FPM,
                    llvm::ArrayRef<llvm::PassBuilder::PipelineElement>
                ) {
                    if (Name == "librarify") {
                        FPM.addPass(Librarify());
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
