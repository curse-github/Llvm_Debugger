#include "controllerLib.h"

bufferWriter::bufferWriter() {
    counter = pointer = nullptr;
    // std::cout << "bufferWriter " << this << " created\n";
}
bufferWriter::bufferWriter(const bufferWriter& copy) {
    // std::cout << "bufferWriter " << &copy << " getting copied1 to " << this << "\n";
    if (copy.pointer == nullptr) {
        counter = pointer = nullptr;
        return;
    }
    pointer = malloc((char*)copy.counter-(char*)copy.pointer);
    std::memcpy(pointer, copy.pointer, (char*)copy.counter-(char*)copy.pointer);
    counter = (void*)((char*)pointer+((char*)copy.counter-(char*)copy.pointer));
}
bufferWriter::bufferWriter(bufferWriter&& move) {
    // std::cout << "bufferWriter " << &move << " getting moved1 to " << this << "\n";
    pointer = move.pointer;
    counter = move.counter;
    move.pointer = nullptr;
    move.counter = nullptr;
}
bufferWriter& bufferWriter::operator=(const bufferWriter& copy) {
    // std::cout << "bufferWriter " << &copy << " getting copied2 to " << this << "\n";
    pointer = malloc((char*)copy.counter-(char*)copy.pointer);
    std::memcpy(pointer, copy.pointer, (char*)copy.counter-(char*)copy.pointer);
    counter = (void*)((char*)pointer+((char*)copy.counter-(char*)copy.pointer));
    return *this;
}
bufferWriter& bufferWriter::operator=(bufferWriter&& move) {
    // std::cout << "bufferWriter " << &move << " getting moved2 to " << this << "\n";
    pointer = move.pointer;
    counter = move.counter;
    move.pointer = nullptr;
    move.counter = nullptr;
    return *this;
}
bufferWriter::~bufferWriter() {
    // std::cout << "bufferWriter " << this << " getting destroyed\n";
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
template <>
void input<char*>(bufferWriter& parameters, std::string paramName, bool doRound) {// override because method of input is different
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
    {"double", input<double>}
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
        if (isStruct) {
            bool isValid = true;
            for (int j = 0; j < structNumFields[i]; j++) {
                if (!isInputableType(structFieldTypes[i][j])) {
                    isValid = false;
                    break;
                }
            }
            return isValid;
        }
        return false;
    }
}
void inputType(std::string type, bufferWriter& parameters, std::vector<bufferWriter*>& storage, std::string paramName, bool doRound) {
    if (type=="char*") {
        std::string tmp;
        std::cout << "Please enter a string for the parameter \"" << paramName << "\" : ";
        std::cin >> tmp;
        size_t i = storage.size();
        storage.push_back(new bufferWriter());
        for(int j = 0; j < tmp.size(); j++)
            storage[i]->push<char>(tmp[j]);
        storage[i]->push<char>('\00');
        parameters.push<void*>(storage[i]->pointer);
    } else if (type[type.size()-1] == '*') {// is an pointer type
        if (type[type.size()-2ull] == ']') {
            size_t str_i = type.find_last_of('[');
            std::string newType = type.substr(0, str_i);
            unsigned int count = std::stoi(type.substr(str_i+1, type.size()-str_i-3));
            size_t i = storage.size();
            storage.push_back(new bufferWriter());
            for (int j = 0; j < count; j++)
                inputType(newType, *storage[i], storage, paramName+'['+std::to_string(j)+']', doRound);
            parameters.push<void*>(storage[i]->pointer);
            return;
        } else {
            size_t i = storage.size();
            storage.push_back(new bufferWriter());
            std::string newType = type.substr(0, type.size()-1);
            std::cout << "Enter number of values for the " << type << ", \"" << paramName << "\" : ";
            unsigned int count = 0;
            std::cin >> count;
            if (count == 0)
                parameters.push<void*>(nullptr);
            else {
                for (int j = 0; j < count; j++)
                    inputType(newType, *storage[i], storage, paramName+'['+std::to_string(j)+']', doRound);
                parameters.push<void*>(storage[i]->pointer);
            }
            return;
        }
    } else if (type[type.size()-1] == ']') {// is an array type
        size_t str_i = type.find_last_of('[');
        std::string newType = type.substr(0, str_i);
        int count = std::stoi(type.substr(str_i+1, type.size()-str_i-2));
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
        if (isStruct) {
            //std::cout << "Creating struct of type \"" << type << "\"\n";
            for (int j = 0; j < structNumFields[i]; j++)
                inputType(structFieldTypes[i][j], parameters, storage, paramName+'.'+structFieldNames[i][j], true);
            return;
        }
    }
}

template <>
void print<bool>(void* ptr, std::ostream& o) {
    o << ((*(bool*)ptr)?"true":"false");
}
template <>
void print<char>(void* ptr, std::ostream& o) {
    char val1 = (*(char*)ptr);
    o << '\'' << val1 << '\'';
}
template <>
void print<char*>(void* ptr, std::ostream& o) {
    const char* cstr = (*(const char**)ptr);
    if (cstr != nullptr) {
        std::string str = cstr;
        size_t loc = str.find('\n');
        while (loc != std::string::npos) {
            str.replace(loc,1,"\\n");
            loc = str.find('\n');
        }
        o << (void*)cstr << " = (c_str)\"" << str << "\\0\"";
    } else
        o << "nullptr";
}
std::map<std::string, printFT> printFunctions = {
    {"bool", print<bool>}, 
    {"char", print<char>}, 
    {"short", print<short>}, 
    {"int", print<int>}, 
    {"long", print<long>}, 
    {"float", print<float>}, 
    {"double", print<double>}, 
    {"char*", print<char*>}
};
unsigned int getLargestTypeSizeContained(std::string type);
void printType(std::string type, void* ptr, std::ostream& o) {
    if (ptr == nullptr) {
        o << "&nullptr";
        return;
    }
    if (printFunctions.count(type) > 0)
        printFunctions[type](ptr, o);
    else if (type[type.size()-1] == '*') {
        if (type == "void*")
            o << ptr;
        else {
            std::string newType = type.substr(0, type.size()-1);
            o << *(void**)ptr << " -> (" << newType << ')';
            printType(newType, *(void**)ptr, o);
        }
    } else if (type[type.size()-1] == ']') {
        size_t str_i = type.find_last_of('[');
        std::string newType = type.substr(0, str_i);
        int count = std::stoi(type.substr(str_i+1, type.size()-str_i-2));
        unsigned int newTypeSize = getTypeByteLength(newType);
        o << "[ ";
        char* runningPtr = (char*)ptr;
        for(int i = 0; i < count; i++) {
            if (i != 0) o << ", ";
            printType(newType, (void*)runningPtr, o);
            runningPtr+=newTypeSize;
        }
        o << " ]";
    } else {
        bool isStruct = false;
        int i;
        for(i = 0; !isStruct&&(i < numStructs); i++)
            if (type == structNames[i]) {
                isStruct = true;
                break;
            }
        if (isStruct) {
            o << "{ ";
            unsigned int offset = 0;
            for (int j = 0; j < structNumFields[i]; j++) {
                if (j != 0) o << ", ";
                unsigned int size = getTypeByteLength(structFieldTypes[i][j]);
                unsigned int largestContained = getLargestTypeSizeContained(structFieldTypes[i][j]);
                offset = offset+(largestContained-offset%largestContained)%largestContained;
                o << structFieldNames[i][j] << "=(" << structFieldTypes[i][j] << ")";
                printType(structFieldTypes[i][j], (void*)((char*)ptr+offset), o);
                offset += size;
            }
            o << " }";
            return;
        }
        bool isUnion = false;
        for(i = 0; !isUnion&&(i < numUnions); i++)
            if (type == unionNames[i]) {
                isUnion = true;
                break;
            }
        if (isUnion) {
            o << "{ ";
            unsigned int offset = 0;
            for (int j = 0; j < unionNumFields[i]; j++) {
                if (j != 0) o << " or ";
                o << unionFieldNames[i][j] << "=(" << unionFieldTypes[i][j] << ")";
                printType(unionFieldTypes[i][j], ptr, o);
            }
            o << " }";
            return;
        }
        o << "unknown";
        return;
    }
}

std::map<std::string, unsigned int> typeByteLengths = {
    {"bool", sizeof(bool)},
    {"char", sizeof(char)},
    {"short", sizeof(short)},
    {"int", sizeof(int)},
    {"long", sizeof(long)},
    {"float", sizeof(float)},
    {"double", sizeof(double)},
};
unsigned int getTypeByteLength(std::string type) {
    if (type[type.size()-1] == '*')
        return sizeof(void*);
    else if (type[type.size()-1] == ']') {
        size_t str_i = type.find_last_of('[');
        std::string newType = type.substr(0, str_i);
        int count = std::stoi(type.substr(str_i+1, type.size()-str_i-2));
        return getTypeByteLength(newType)*count;
    } else if (typeByteLengths.count(type) > 0)
        return typeByteLengths[type];
    else {
        bool isStruct = false;
        int i;
        for(i = 0; !isStruct&&(i < numStructs); i++)
            if (type == structNames[i]) {
                isStruct = true;
                break;
            }
        if (isStruct) {
            unsigned int totalLength = 0;
            for (int j = 0; j < structNumFields[i]; j++) {
                unsigned int length = getTypeByteLength(structFieldTypes[i][j]);
                totalLength = totalLength+(length-totalLength%length)%length;
                totalLength += length;
            }
            return totalLength;
        }
        bool isUnion = false;
        for(i = 0; !isUnion&&(i < numUnions); i++)
            if (type == unionNames[i]) {
                isUnion = true;
                break;
            }
        if (isUnion) {
            unsigned int largestSize = 0;
            for (int j = 0; j < structNumFields[i]; j++) {
                unsigned int size = getTypeByteLength(structFieldTypes[i][j]);
                if (size > largestSize) largestSize = size;
            }
            return largestSize;
        }
        return 0;
    }
    return 0;
}
unsigned int getLargestTypeSizeContained(std::string type) {
    if (type[type.size()-1] == '*')
        return sizeof(void*);
    else if (type[type.size()-1] == ']') {
        size_t str_i = type.find_last_of('[');
        std::string newType = type.substr(0, str_i);
        return getTypeByteLength(newType);
    } else if (typeByteLengths.count(type) > 0)
        return typeByteLengths[type];
    else {
        bool isStruct = false;
        int i;
        for(i = 0; !isStruct&&(i < numStructs); i++)
            if (type == structNames[i]) {
                isStruct = true;
                break;
            }
        if (isStruct) {
            unsigned int largestSize = 0;
            for (int j = 0; j < structNumFields[i]; j++) {
                unsigned int size = getLargestTypeSizeContained(structFieldTypes[i][j]);
                if (size > largestSize) largestSize = size;
            }
            return largestSize;
        }
        bool isUnion = false;
        for(i = 0; !isUnion&&(i < numUnions); i++)
            if (type == unionNames[i]) {
                isUnion = true;
                break;
            }
        if (isUnion) {
            unsigned int largestSize = 0;
            for (int j = 0; j < structNumFields[i]; j++) {
                unsigned int size = getLargestTypeSizeContained(structFieldTypes[i][j]);
                if (size > largestSize) largestSize = size;
            }
            return largestSize;
        }
        return 0;
    }
    return 0;
}