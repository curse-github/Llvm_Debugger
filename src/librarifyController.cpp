#include "controllerLib.h"

int main(int argc, char** argv) {
    std::vector<bool> isFunctionValid;
    std::vector<bool> canPrintFunctionOutput;
    size_t numValid = 0ull;
    for (unsigned int i = 0; i < numFunctions; i++) {
        const std::string functionReturnType = functionReturnTypes[i];
        bool isValid = true;
        unsigned int paramCount = functionParamCounts[i];
        for(int j = 0; isValid && (j < paramCount); j++)
            if (!isInputableType(functionParamTypes[i][j]))
                isValid = false;
        isFunctionValid.push_back(isValid);
        canPrintFunctionOutput.push_back((functionReturnType != "unknown") && (functionReturnType[functionReturnType.size()-1] != '*'));
        if (isValid) numValid++;
    }
    //* 
    if (numValid != numFunctions) {
        std::cout << "Can call " << numValid << " out of " << numFunctions << " functions.\n";
        std::cout << "Invalid functions are as follows:\n";
    }
    for(size_t i = 0; i < numFunctions; i++) {
        if (isFunctionValid[i]) continue;
        std::cout << "  " << (i+1) << ": " << functionReturnTypes[i] << ' ' << functionNames[i] << '(';
        unsigned int paramCount = functionParamCounts[i];
        for(int j = 0; j < paramCount; j++)
            std::cout << functionParamTypes[i][j] << ' ' << functionParamNames[i][j] << ((j!=(paramCount-1))?", ":"");
        std::cout << ")\n";
    }//*/
    /*
    std::cout << "struct types {\n";
    for(int i = 0; i < numStructs; i++) {
        std::cout << "    " << structNames[i] << " : {\n";
        for (int j = 0; j < structNumContainedTypes[i]; j++) {
            std::cout << "        " << structContainedTypes[i][j] << '\n';
        }
        std::cout << "    }\n";
    }
    std::cout << "}\n";//*/

    char choice = 'Y';
    while ((choice == 'Y') || (choice == 'y')) {
        std::cout << "Pick a function from the following:\n";
        for(unsigned int i = 0; i < numFunctions; i++) {
            if (!isFunctionValid[i]) continue;
            std::cout << "  " << (i+1) << ": " << functionReturnTypes[i] << ' ' << functionNames[i] << '(';
            unsigned int paramCount = functionParamCounts[i];
            for(int j = 0; j < paramCount; j++)
                std::cout << functionParamTypes[i][j] << ' ' << functionParamNames[i][j] << ((j!=(paramCount-1))?", ":"");
            std::cout << ")\n";
        }
        std::cout << "Enter your choice: ";
        unsigned int i = 0;
        std::cin >> i;
        while (i < 1 || i > numFunctions || !isFunctionValid[i-1]) {
            std::cout << "Invalid choice.\nRe-enter your choice: ";
            std::cin >> i;
        }
        i--;

        std::string functionName = functionNames[i];
        std::cout << "\nYou chose the function \"";
        const std::string functionReturnType = functionReturnTypes[i];
        std::cout << functionReturnType << ' ' << functionNames[i] << '(';
        unsigned int paramCount = functionParamCounts[i];
        for(int j = 0; j < paramCount; j++)
            std::cout << functionParamTypes[i][j] << ' ' << functionParamNames[i][j] << ((j!=(paramCount-1))?", ":"");
        std::cout << ")\"\n";

        bufferWriter parameters;
        std::vector<bufferWriter*> storage;
        for(int j = 0; j < paramCount; j++)
            inputType(functionParamTypes[i][j], parameters, storage, functionParamNames[i][j], false);
        if (canPrintFunctionOutput[i]) {
            if (std::strcmp(functionReturnType.c_str(), "bool") == 0) {
                bool output = ((boolFT)functionPointers[i])(parameters.pointer);
                std::cout << '\"' << functionName << "\" output = (bool)" << (output?"true":"false") << '\n';
            } else if (std::strcmp(functionReturnType.c_str(), "char") == 0) {
                char output = ((charFT)functionPointers[i])(parameters.pointer);
                std::cout << '\"' << functionName << "\" output = (char)'" << output << "'\n";
            } else if (std::strcmp(functionReturnType.c_str(), "short") == 0) {
                short output = ((shortFT)functionPointers[i])(parameters.pointer);
                std::cout << '\"' << functionName << "\" output = (short)" << output << '\n';
            } else if (std::strcmp(functionReturnType.c_str(), "int") == 0) {
                int output = ((intFT)functionPointers[i])(parameters.pointer);
                std::cout << '\"' << functionName << "\" output = (int)" << output << '\n';
            } else if (std::strcmp(functionReturnType.c_str(), "long") == 0) {
                long output = ((longFT)functionPointers[i])(parameters.pointer);
                std::cout << '\"' << functionName << "\" output = (long)" << output << '\n';
            } else if (std::strcmp(functionReturnType.c_str(), "float") == 0) {
                float output = ((floatFT)functionPointers[i])(parameters.pointer);
                std::cout << '\"' << functionName << "\" output = (float)" << output << '\n';
            } else if (std::strcmp(functionReturnType.c_str(), "double") == 0) {
                double output = ((doubleFT)functionPointers[i])(parameters.pointer);
                std::cout << '\"' << functionName << "\" output = (double)" << output << '\n';
            } else 
                functionPointers[i](parameters.pointer);
        } else 
            functionPointers[i](parameters.pointer);
        for(int j = 0; j < storage.size(); j++)
            delete storage[j];
        std::cout << "\nwould you like to run another function? (Y/N) : ";
        std::cin >> choice;
        std::cout << '\n';
    }
    return 0;
}