#include "controllerLib.h"

std::ostream* o = &std::cout;
unsigned int indentLevel = 0;
extern "C" void logFunctionParameters(const char* funcName, void* buffer) {
    for(int i = 0; i < indentLevel; i++) *o << "    ";
    indentLevel++;
    for(int j = 0; j < numFunctions; j++) {
        if (std::strcmp(funcName, functionMangledNames[j]) == 0) {
            unsigned int paramCount = functionParamCounts[j];
            *o << "Function \"" << functionNames[j] << "\" was called";
            if (paramCount > 0) {
                *o << " with parameters {\n";
                unsigned int offset = 0;
                for(int k = 0; k < paramCount; k++) {
                    for(int i = 0; i < indentLevel; i++) *o << "    ";
                    *o << functionParamNames[j][k] << " = (" << functionParamTypes[j][k] << ")";
                    printType(functionParamTypes[j][k], (void*)((char*)buffer+offset), *o);
                    offset += getTypeByteLength(functionParamTypes[j][k]);
                    *o << "\n";
                }
                for(int i = 1; i < indentLevel; i++) *o << "    ";
                *o << '}';
            }
            *o << '\n';
        }
    }
}
extern "C" void logFunctionReturn(const char* funcName, void* buffer) {
    indentLevel--;
    for(int i = 0; i < indentLevel; i++) *o << "    ";
    for(int j = 0; j < numFunctions; j++) {
        if (std::strcmp(funcName, functionMangledNames[j]) == 0) {
            *o << "Function \"" << functionNames[j] << "\" returned";
            if (std::strcmp(functionReturnTypes[j], "void") != 0) {
                *o << ", output = (" << functionReturnTypes[j] << ")";
                printType(functionReturnTypes[j], buffer, *o);
            }
            *o << "\n";
        }
    }
}

int main(int argc, char** argv) {
    for (int i = 0; i < argc; i++)
        std::cout << '"' << argv[i] << "\"\n";
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
    if (!isValid) {
        std::cout << "Invalid main function\n";
        return 1;
    }
    // open file for output
    std::fstream* f;
    f = new std::fstream();
    f->open("out/output.txt", std::ios::out);
    //o = f;
    // get parameters for main
    bufferWriter parameters;
    std::vector<bufferWriter*> storage;
    for(int j = 0; j < functionParamCounts[i]; j++)
        inputType(functionParamTypes[i][j], parameters, storage, functionParamNames[i][j], false);
    // call main
    logFunctionParameters("main", parameters.pointer);
    if (std::strcmp(functionReturnTypes[i], "int") == 0) {
        int output = ((intFT)functionPointers[i])(parameters.pointer);
        logFunctionReturn("main", (void*)&output);
    } else {
        functionPointers[i](parameters.pointer);
        logFunctionReturn("main", nullptr);
    }
    for(int j = 0; j < storage.size(); j++)
        delete storage[j];
    f->close();
    // somehow, this doesnt work any other way.
    delete f;
    return 0;
}