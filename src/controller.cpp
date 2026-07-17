#include <cstring>
#include <iostream>
#include <vector>
#include <memory>
#include <map> 

class bufferWriter {
private:
    void* counter;
public:
    void* pointer;
    bufferWriter() {
        counter = pointer = malloc(0);
    }
    ~bufferWriter() {
        free(pointer);
        pointer = nullptr;
        counter = nullptr;
    }
    template<typename T>
    void push(T val) {
        void* old = pointer;
        pointer = malloc((char*)counter-(char*)old+sizeof(T));
        std::memcpy(pointer,old,(char*)counter-(char*)old);
        counter=(void*)((char*)counter-(char*)old+(char*)pointer);
        *(T*)counter = val;
        counter=(void*)((char*)counter+sizeof(T));
    }
    template<>
    void push<void*>(void* val) {
        void* old = pointer;
        pointer = malloc((char*)counter-(char*)old+sizeof(void*));
        std::memcpy(pointer,old,(char*)counter-(char*)old);
        counter=(void*)((char*)counter-(char*)old+(char*)pointer);
        *(void**)counter = val;
        counter=(void*)((char*)counter+sizeof(void*));
    }
};

extern unsigned int numFunctions;
extern const char* functionNames[];
extern const char* functionReturnTypes[];
extern unsigned int functionParamCounts[];
extern const char** functionParamNames[];
extern const char** functionParamTypes[];
typedef void(*wrapperFT)(void*);
typedef bool(*boolFT)(void*);
typedef char(*charFT)(void*);
typedef short(*shortFT)(void*);
typedef int(*intFT)(void*);
typedef long(*longFT)(void*);
typedef float(*floatFT)(void*);
typedef double(*doubleFT)(void*);
extern wrapperFT functionPointers[];

extern unsigned int numStructs;
extern const char* structNames[];

void inputBool(bufferWriter* parameters, std::string paramName) {
    std::string tmp;
    while (true) {
        std::cout << "Please enter a bool for the parameter \"" << paramName << "\" : ";
        std::cin >> tmp;
        if (std::strcmp(tmp.c_str(), "true") == 0) {
            parameters->push<bool>(true);
            break;
        } else if (std::strcmp(tmp.c_str(), "false") == 0) {
            parameters->push<bool>(false);
            break;
        } else
            std::cout << "Invalid value try again.\n";
    }
}
void inputChar(bufferWriter* parameters, std::string paramName) {
    char tmp;
    std::cout << "Please enter a char for the parameter \"" << paramName << "\" : ";
    std::cin >> tmp;
    parameters->push<char>(tmp);
}
void inputShort(bufferWriter* parameters, std::string paramName) {
    short tmp;
    std::cout << "Please enter a short for the parameter \"" << paramName << "\" : ";
    std::cin >> tmp;
    parameters->push<short>(tmp);
}
void inputInt(bufferWriter* parameters, std::string paramName) {
    int tmp;
    std::cout << "Please enter an int for the parameter \"" << paramName << "\" : ";
    std::cin >> tmp;
    parameters->push<int>(tmp);
}
void inputLong(bufferWriter* parameters, std::string paramName) {
    long tmp;
    std::cout << "Please enter a long for the parameter \"" << paramName << "\" : ";
    std::cin >> tmp;
    parameters->push<long>(tmp);
}
void inputFloat(bufferWriter* parameters, std::string paramName) {
    float tmp;
    std::cout << "Please enter a float for the parameter \"" << paramName << "\" : ";
    std::cin >> tmp;
    parameters->push<float>(tmp);
}
void inputDouble(bufferWriter* parameters, std::string paramName) {
    double tmp;
    std::cout << "Please enter a double for the parameter \"" << paramName << "\" : ";
    std::cin >> tmp;
    parameters->push<double>(tmp);
}
void inputCStr(bufferWriter* parameters, std::string paramName) {
    std::string tmp;
    std::cout << "Please enter a string for the parameter \"" << paramName << "\" : ";
    std::cin >> tmp;
    parameters->push<void*>((void*)tmp.c_str());
}
typedef void(*inputFT)(bufferWriter*, std::string);
std::map<std::string, inputFT> inputFunctions = {
    {"bool", inputBool},
    {"char", inputChar},
    {"short", inputShort},
    {"int", inputInt},
    {"long", inputLong},
    {"float", inputFloat},
    {"double", inputDouble},
    {"char*", inputCStr}
};

int main(int argc, char** argv) {
    std::vector<unsigned int> valid_indices;
    size_t numValid = 0ull;
    for (unsigned int i = 0; i < numFunctions; i++) {
        const std::string functionReturnType = functionReturnTypes[i];
        bool valid = (functionReturnType != "unknown") && (functionReturnType[functionReturnType.size()-1] != '*');
        unsigned int paramCount = functionParamCounts[i];
        for(int j = 0; valid && (j < paramCount); j++) {
            const std::string functionParamType = functionParamTypes[i][j];
            if (inputFunctions.count(functionParamType) <= 0)
                valid = false;
        }
        if (valid) {
            valid_indices.push_back(i);
            numValid++;
        }
    }
    std::cout << "Can call " << numValid << " out of " << numFunctions << " functions.\n";
    /*std::cout << "struct types {\n";
    for(int i = 0; i < numStructs; i++) {
        std::cout << "    " << structNames[i] << '\n';
    }
    std::cout << "}\n";//*/

    char choice = 'Y';
    while ((choice == 'Y') || (choice == 'y')) {
        std::cout << "Pick a function from the following:\n";
        for(size_t index = 0; index < numValid; index++) {
            unsigned int i = valid_indices[index];
            std::cout << "  " << (index+1) << ": " << functionReturnTypes[i] << ' ' << functionNames[i] << '(';
            unsigned int paramCount = functionParamCounts[i];
            for(int j = 0; j < paramCount; j++)
                std::cout << functionParamTypes[i][j] << ' ' << functionParamNames[i][j] << ((j!=(paramCount-1))?", ":"");
            std::cout << ")\n";
        }
        unsigned int index = 0;
        std::cout << "Enter your choice: ";
        std::cin >> index;
        while (index < 1 || index > numValid) {
            std::cout << "Invalid choice.\nRe-enter your choice: ";
        std::cin >> index;
        }
        unsigned int i = valid_indices[static_cast<size_t>(index-1)];

        std::string functionName = functionNames[i];
        std::cout << "\nYou chose the function \"";
        const std::string functionReturnType = functionReturnTypes[i];
        std::cout << functionReturnType << ' ' << functionNames[i] << '(';
        unsigned int paramCount = functionParamCounts[i];
        for(int j = 0; j < paramCount; j++)
            std::cout << functionParamTypes[i][j] << ' ' << functionParamNames[i][j] << ((j!=(paramCount-1))?", ":"");
        std::cout << ")\"\n";

        bufferWriter parameters;
        for(int j = 0; j < paramCount; j++)
            inputFunctions[paramType](&functionParamTypes[i][j], functionParamNames[i][j]);
        
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
        std::cout << "\nwould you like to run another function? (Y/N) : ";
        std::cin >> choice;
        std::cout << '\n';
    }
    return 0;
}