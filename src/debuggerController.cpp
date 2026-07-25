#include "controllerLib.h"

unsigned int indentLevel = 0;
int main(int argc, char** argv) {
    // get index of main function and whether it is valid
    int i;
    bool isValid = true;
    for (i = 0; i < numFunctions; i++) {
        if (std::strcmp(functionNames[i], "main") != 0)
            continue;
        for(int j = 0; isValid && (j < functionParamCounts[i]); j++)
            if (!isInputableType(functionParamTypes[i][j]))
                isValid = false;
        break;
    }
    // get parameters for main
    bufferWriter parameters;
    std::vector<bufferWriter> storage;
    for(int j = 0; j < functionParamCounts[i]; j++)
        inputType(functionParamTypes[i][j], parameters, storage, functionParamNames[i][j], false);
    // call main
    if (std::strcmp(functionReturnTypes[i], "int") == 0) {
        std::cout << "Function \"main\" was called\n";
        indentLevel++;
        int output = ((intFT)functionPointers[i])(parameters.pointer);
        std::cout << "Function \"main\" returned\n";
        std::cout << "exit code = (int)" << output << '\n';
    } else 
        functionPointers[i](parameters.pointer);
    return 0;
}
extern "C" void logFunctionParameters(const char* funcName, void* buffer) {
    for(int i = 0; i < indentLevel; i++) std::cout << "    ";
    std::cout << "Function \"" << funcName << "\" was called";
    indentLevel++;
    for(int j = 0; j < numFunctions; j++) {
        if (std::strcmp(funcName, functionNames[j]) == 0) {
            unsigned int paramCount = functionParamCounts[j];
            if (paramCount > 0) {
                std::cout << " with parameters {\n";
                for(int k = 0; k < paramCount; k++) {
                    for(int i = 0; i < indentLevel; i++) std::cout << "    ";
                    std::cout << functionParamNames[j][k] << " = (" << functionParamTypes[j][k] << ")" << 0 << "\n";
                }
                for(int i = 1; i < indentLevel; i++) std::cout << "    ";
                std::cout << '}';
            }
            std::cout << '\n';
        }
    }
}
extern "C" void logFunctionReturn(const char* funcName, void* buffer) {
    indentLevel--;
    for(int i = 0; i < indentLevel; i++) std::cout << "    ";
    std::cout << "Function \"" << funcName << "\" returned\n";
}