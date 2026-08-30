#include "controllerLib.h"

std::ostream* o = &std::cout;
extern "C" void logFunctionParameters(const char* funcName, void* buffer) {
    if (o == nullptr) return;
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
                if (functionIsVariadic[j]) {
                    for(int i = 0; i < indentLevel; i++) *o << "    ";
                    int n = *(int*)((char*)buffer+offset);
                    offset += sizeof(int);
                    if (n > 0) {
                        *o << "... = (" << n << " * T) {\n";
                        indentLevel++;
                        for(int k = 0; k < n; k++) {
                            for(int i = 0; i < indentLevel; i++) *o << "    ";
                            char* type = *(char**)((char*)buffer+offset);
                            *o << '(' << type << ") ";
                            offset += sizeof(void*);
                            printType(type, (void*)((char*)buffer+offset), *o);
                            if (k+1!=n) *o << ',';
                            *o << "\n";
                            offset += getTypeByteLength(type);
                        }
                        indentLevel--;
                        for(int i = 0; i < indentLevel; i++) *o << "    ";
                        *o << "}\n";
                    } else {
                        *o << "... = (0 * T) { }\n";
                    }
                }
                for(int i = 1; i < indentLevel; i++) *o << "    ";
                *o << '}';
            } else {
                *o << " without parameters";
            }
            *o << '\n';
            return;
        }
    }
    *o << "Function \"" << funcName << "\" was called with unknown parameters\n";
}
extern "C" void logFunctionReturn(const char* funcName, void* buffer) {
    if (o == nullptr) return;
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
            return;
        }
    }
    *o << "Function \"" << funcName << "\" returned\n";
}

int main(int argc, char** argv) {
    /*
    std::cout << "struct types {\n";
    for(int i = 0; i < numStructs; i++) {
        std::cout << "    " << structNames[i] << " : {\n";
        for (int j = 0; j < structNumFields[i]; j++) {
            std::cout << "        " << structFieldTypes[i][j] << ' ' << structFieldNames[i][j] << '\n';
        }
        std::cout << "    }\n";
    }
    std::cout << "}\n";
    std::cout << "enum types {\n";
    for(int i = 0; i < numEnums; i++) {
        std::cout << "    " << enumNames[i] << " = " << enumTypes[i] << " : {\n";
        for (int j = 0; j < enumNumValues[i]; j++) {
            std::cout << "        " << enumValueNames[i][j] << " = " << enumValueValues[i][j] << '\n';
        }
        std::cout << "    }\n";
    }
    std::cout << "}\n";//*/
    //*
    // get index of main function and whether it is valid
    int i;
    bool isValid = true;
    for (i = 0; i < numFunctions; i++) {
        if (std::strcmp(functionNames[i], "main") != 0)
            continue;
        for(int j = 0; j < functionParamCounts[i]; j++)
            if (!isInputableType(functionParamTypes[i][j])) {
                isValid = false;
                break;
            }
        break;
    }
    if (!isValid) {
        std::cout << "Invalid main function\n";
        return 1;
    }
    // open file for output
    std::fstream f;
    f.open("out/output.txt", std::ios::out);
    o = &f;
    // get parameters for main
    bufferWriter parameters;
    std::vector<bufferWriter*> storage;
    if (argc > 1) {
        if (functionParamCounts[i] == 2) {
            parameters.push<int>(argc);
            storage.push_back(new bufferWriter());
            for(unsigned int j = 0; j < argc; j++)
                storage[0]->push<void*>((void*)argv[j]);
            parameters.push<void*>((void*)storage[0]->pointer);
        }
    } else
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
    o = nullptr;
    f.close();
    // cleanup
    for(int j = 0; j < storage.size(); j++)
        delete storage[j];
    return 0;
}