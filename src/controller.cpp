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
        counter = pointer = nullptr;
    }
    bufferWriter(const bufferWriter& copy) {
        pointer = malloc((char*)copy.counter-(char*)copy.pointer);
        std::memcpy(pointer,copy.pointer,(char*)copy.counter-(char*)copy.pointer);
        counter = (void*)((char*)pointer+((char*)copy.counter-(char*)copy.pointer));
    }
    bufferWriter(bufferWriter&& move) {
        pointer = move.pointer;
        counter = move.counter;
        move.pointer = nullptr;
        move.counter = nullptr;
    }
    bufferWriter& operator=(const bufferWriter& copy) {
        pointer = malloc((char*)copy.counter-(char*)copy.pointer);
        std::memcpy(pointer,copy.pointer,(char*)copy.counter-(char*)copy.pointer);
        counter = (void*)((char*)pointer+((char*)copy.counter-(char*)copy.pointer));
        return *this;
    }
    bufferWriter& operator=(bufferWriter&& move) {
        pointer = move.pointer;
        counter = move.counter;
        move.pointer = nullptr;
        move.counter = nullptr;
        return *this;
    }
    ~bufferWriter() {
        if (pointer != nullptr) free(pointer);
        pointer = nullptr;
        counter = nullptr;
    }
    template<typename T>
    void push(T val) {
        // resere new space
        void* old = pointer;
        pointer = malloc((char*)counter-(char*)old+sizeof(T));
        std::memcpy(pointer,old,(char*)counter-(char*)old);
        counter=(void*)((char*)pointer+((char*)counter-(char*)old));
        // free old space
        free(old);
        // push new value
        *((T*)counter) = val;
        counter=(void*)((char*)counter+sizeof(T));
    }
    template<>
    void push<void*>(void* val) {
        // resere new space
        void* old = pointer;
        pointer = malloc((char*)counter-(char*)old+sizeof(void*));
        std::memcpy(pointer,old,(char*)counter-(char*)old);
        counter=(void*)((char*)pointer+((char*)counter-(char*)old));
        // free old space
        free(old);
        // push new value
        *((void**)counter) = val;
        counter=(void*)((char*)counter+sizeof(void*));
    }
    void roundToMultipleOf(int amount) {
        // round size
        int oldCount = (int)((char*)counter-(char*)pointer);
        int count = oldCount+(amount-oldCount%amount)%amount;
        if (count == oldCount) return;
        // resere new space
        void* old = pointer;
        pointer = malloc((char*)counter-(char*)old+(count-oldCount));
        std::memcpy(pointer,old,(char*)counter-(char*)old);
        counter=(void*)((char*)pointer+((char*)counter-(char*)old));
        // free old space
        free(old);
        // zero out new space
        for (int i = 0; i < (count-oldCount); i++) {
            *(char*)counter = '\00';
            counter = (void*)((char*)counter+1);
        }
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
extern unsigned int structNumContainedTypes[];
extern const char** structContainedTypes[];

void inputBool(bufferWriter& parameters, std::string paramName, bool doRound) {
    std::string tmp;
    while (true) {
        std::cout << "Please enter a bool for the parameter \"" << paramName << "\" : ";
        std::cin >> tmp;
        if (std::strcmp(tmp.c_str(), "true") == 0) {
            parameters.push<bool>(true);
            break;
        } else if (std::strcmp(tmp.c_str(), "false") == 0) {
            parameters.push<bool>(false);
            break;
        } else
            std::cout << "Invalid value try again.\n";
    }
}
void inputChar(bufferWriter& parameters, std::string paramName, bool doRound) {
    char tmp;
    std::cout << "Please enter a char for the parameter \"" << paramName << "\" : ";
    std::cin >> tmp;
    parameters.push<char>(tmp);
}
void inputShort(bufferWriter& parameters, std::string paramName, bool doRound) {
    short tmp;
    std::cout << "Please enter a short for the parameter \"" << paramName << "\" : ";
    std::cin >> tmp;
    parameters.push<short>(tmp);
}
void inputInt(bufferWriter& parameters, std::string paramName, bool doRound) {
    int tmp;
    std::cout << "Please enter an int for the parameter \"" << paramName << "\" : ";
    std::cin >> tmp;
    parameters.roundToMultipleOf(sizeof(int));
    parameters.push<int>(tmp);
}
void inputLong(bufferWriter& parameters, std::string paramName, bool doRound) {
    long tmp;
    std::cout << "Please enter a long for the parameter \"" << paramName << "\" : ";
    std::cin >> tmp;
    parameters.roundToMultipleOf(sizeof(long));
    parameters.push<long>(tmp);
}
void inputFloat(bufferWriter& parameters, std::string paramName, bool doRound) {
    float tmp;
    std::cout << "Please enter a float for the parameter \"" << paramName << "\" : ";
    std::cin >> tmp;
    parameters.roundToMultipleOf(sizeof(float));
    parameters.push<float>(tmp);
}
void inputDouble(bufferWriter& parameters, std::string paramName, bool doRound) {
    double tmp;
    std::cout << "Please enter a double for the parameter \"" << paramName << "\" : ";
    std::cin >> tmp;
    parameters.roundToMultipleOf(sizeof(double));
    parameters.push<double>(tmp);
}
void inputCStr(bufferWriter& parameters, std::string paramName, bool doRound) {
    std::string tmp;
    std::cout << "Please enter a string for the parameter \"" << paramName << "\" : ";
    std::cin >> tmp;
    parameters.roundToMultipleOf(sizeof(void*));
    parameters.push<void*>((void*)tmp.c_str());
}
typedef void(*inputFT)(bufferWriter&, std::string, bool);
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
bool isInputableType(std::string type) {
    if (type[type.size()-1] == '*') {// is an pointer type
        return isInputableType(type.substr(0,type.size()-1));
    } else if (type[type.size()-1] == ']') {// is an array type
        return false;
    } else if (inputFunctions.count(type) > 0)
        return true;
    else {
        bool isStruct = false;
        int i;
        for(i = 0; !isStruct&&(i < numStructs); i++) {
            if (type == structNames[i]) {
                isStruct = true;
                break;
            }
        }
        if (!isStruct)
            return false;
        bool isValid = true;
        for (int j = 0; j < structNumContainedTypes[i]; j++) {
            if (!isInputableType(structContainedTypes[i][j])) {
                isValid = false;
                break;
            }
        }
        return isValid;
    }
}
void inputType(std::string type, bufferWriter& parameters, std::vector<bufferWriter>& storage, std::string paramName, bool doRound=false) {
    if (type=="char*")
        inputFunctions[type](parameters, paramName, doRound);
    else if (type[type.size()-1] == '*') {// is an pointer type
        size_t i = storage.size();
        storage.push_back((bufferWriter&&)bufferWriter());
        inputType(type.substr(0,type.size()-1), storage[i], storage, "*"+paramName);
        parameters.push<void*>(storage[i].pointer);
        return;
    } else if (type[type.size()-1] == ']')// is an array type
        return;
    else if (inputFunctions.count(type) > 0)
        inputFunctions[type](parameters, paramName, doRound);
    else {
        bool isStruct = false;
        int i;
        for(i = 0; !isStruct&&(i < numStructs); i++) {
            if (type == structNames[i]) {
                isStruct = true;
                break;
            }
        }
        if (!isStruct)
            return;
        for (int j = 0; j < structNumContainedTypes[i]; j++) {
            inputType(structContainedTypes[i][j], parameters, storage, paramName+'['+std::to_string(j)+']', true);
        }
        return;
    }
}

int main(int argc, char** argv) {
    std::vector<unsigned int> valid_indices;
    size_t numValid = 0ull;
    for (unsigned int i = 0; i < numFunctions; i++) {
        const std::string functionReturnType = functionReturnTypes[i];
        bool isValid = (functionReturnType != "unknown") && (functionReturnType[functionReturnType.size()-1] != '*');
        unsigned int paramCount = functionParamCounts[i];
        for(int j = 0; isValid && (j < paramCount); j++)
            if (!isInputableType(functionParamTypes[i][j]))
                isValid = false;
        if (isValid) {
            valid_indices.push_back(i);
            numValid++;
        }
    }
    std::cout << "Can call " << numValid << " out of " << numFunctions << " functions.\n";
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
        std::vector<bufferWriter> storage;
        for(int j = 0; j < paramCount; j++)
            inputType(functionParamTypes[i][j], parameters, storage, functionParamNames[i][j]);
        
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