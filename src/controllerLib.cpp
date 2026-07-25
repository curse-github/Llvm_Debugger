#include "controllerLib.h"

bufferWriter::bufferWriter() {
    counter = pointer = nullptr;
}
bufferWriter::bufferWriter(const bufferWriter& copy) {
    pointer = malloc((char*)copy.counter-(char*)copy.pointer);
    std::memcpy(pointer, copy.pointer, (char*)copy.counter-(char*)copy.pointer);
    counter = (void*)((char*)pointer+((char*)copy.counter-(char*)copy.pointer));
}
bufferWriter::bufferWriter(bufferWriter&& move) {
    pointer = move.pointer;
    counter = move.counter;
    move.pointer = nullptr;
    move.counter = nullptr;
}
bufferWriter& bufferWriter::operator=(const bufferWriter& copy) {
    pointer = malloc((char*)copy.counter-(char*)copy.pointer);
    std::memcpy(pointer, copy.pointer, (char*)copy.counter-(char*)copy.pointer);
    counter = (void*)((char*)pointer+((char*)copy.counter-(char*)copy.pointer));
    return *this;
}
bufferWriter& bufferWriter::operator=(bufferWriter&& move) {
    pointer = move.pointer;
    counter = move.counter;
    move.pointer = nullptr;
    move.counter = nullptr;
    return *this;
}
bufferWriter::~bufferWriter() {
    if (pointer != nullptr) free(pointer);
    pointer = nullptr;
    counter = nullptr;
}
void bufferWriter::roundToMultipleOf(int amount) {
    // round size
    int oldCount = (int)((char*)counter-(char*)pointer);
    int count = oldCount+(amount-oldCount%amount)%amount;
    if (count == oldCount) return;
    // resere new space
    void* old = pointer;
    pointer = malloc((char*)counter-(char*)old+(count-oldCount));
    std::memcpy(pointer, old, (char*)counter-(char*)old);
    counter=(void*)((char*)pointer+((char*)counter-(char*)old));
    // free old space
    free(old);
    // zero out new space
    for (int i = 0; i < (count-oldCount); i++) {
        *(char*)counter = '\00';
        counter = (void*)((char*)counter+1);
    }
}
int bufferWriter::getSize() {
    return (int)((char*)counter-(char*)pointer);
}

template <>
void input<int>(bufferWriter& parameters, std::string paramName, bool doRound) {// override to replace "a" with "an"
    int tmp;
    std::cout << "Please enter an int for the parameter \"" << paramName << "\" : ";
    std::cin >> tmp;
    if (doRound) parameters.roundToMultipleOf(sizeof(int));
    parameters.push<int>(tmp);
}
template <>
void input<bool>(bufferWriter& parameters, std::string paramName, bool doRound) {// override because method of input is different
    std::string tmp;
    // dont bother rounder to the nearest single byte
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
void inputCStr(bufferWriter& parameters, std::string paramName, bool doRound) {// override because method of input is different
    std::string tmp;
    std::cout << "Please enter a string for the parameter \"" << paramName << "\" : ";
    std::cin >> tmp;
    parameters.roundToMultipleOf(sizeof(void*));
    parameters.push<void*>((void*)tmp.c_str());
}
std::map<std::string, inputFT> inputFunctions = {
    {"bool", input<bool>}, 
    {"char", input<char>}, 
    {"short", input<short>}, 
    {"int", input<int>}, 
    {"long", input<long>}, 
    {"float", input<float>}, 
    {"double", input<double>}, 
    {"char*", inputCStr}
};
bool isInputableType(std::string type) {
    if (type[type.size()-1] == '*') {// is an pointer type
        return isInputableType(type.substr(0, type.size()-1));
    } else if (type[type.size()-1] == ']') {// is an array type
        return isInputableType(type.substr(0, type.find_last_of('[')));
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
void inputType(std::string type, bufferWriter& parameters, std::vector<bufferWriter>& storage, std::string paramName, bool doRound) {
    if (type=="char*")
        inputFunctions[type](parameters, paramName, doRound);
    else if (type[type.size()-1] == '*') {// is an pointer type
        size_t i = storage.size();
        storage.push_back((bufferWriter&&)bufferWriter());
        std::string newType = type.substr(0, type.size()-1);
        //std::cout << "Creating pointer of type \"" << newType << "*\"\n";
        inputType(newType, storage[i], storage, "(*"+paramName+")", doRound);
        parameters.push<void*>(storage[i].pointer);
        return;
    } else if (type[type.size()-1] == ']') {// is an array type
        size_t str_i = type.find_last_of('[');
        std::string newType = type.substr(0, str_i);
        int count = std::stoi(type.substr(str_i+1, type.size()-str_i-2));
        //std::cout << "Creating array of type \"" << newType << "[" << count << "]\"\n";
        for (int i = 0; i < count; i++)
            inputType(newType, parameters, storage, paramName+'['+std::to_string(i)+']', false);
        return;
    } else if (inputFunctions.count(type) > 0)
        inputFunctions[type](parameters, paramName, doRound);
    else {
        bool isStruct = false;
        int i;
        for(i = 0; !isStruct&&(i < numStructs); i++)
            if (type == structNames[i]) {
                isStruct = true;
                break;
            }
        if (!isStruct)
            return;
        //std::cout << "Creating struct of type \"" << type << "\"\n";
        for (int j = 0; j < structNumContainedTypes[i]; j++)
            inputType(structContainedTypes[i][j], parameters, storage, paramName+'['+std::to_string(j)+']', true);
        return;
    }
}