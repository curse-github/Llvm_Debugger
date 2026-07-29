#ifndef __CONTROLLER
#define __CONTROLLER

#include <cstring>
#include <iostream>
#include <vector>
#include <memory>
#include <map>
#include <fstream>

template<typename T> struct TypeName { static const char *Get() { return ""; }};
#define ENABLE_TYPENAME(A) template<> struct TypeName<A> { static const char *Get() { return #A; }};
ENABLE_TYPENAME(char)
ENABLE_TYPENAME(short)
ENABLE_TYPENAME(int)
ENABLE_TYPENAME(long)
ENABLE_TYPENAME(float)
ENABLE_TYPENAME(double)

class bufferWriter {
private:
public:
    void* counter;
    void* pointer;
    bufferWriter();
    bufferWriter(const bufferWriter& copy);
    bufferWriter(bufferWriter&& move);
    bufferWriter& operator=(const bufferWriter& copy);
    bufferWriter& operator=(bufferWriter&& move);
    ~bufferWriter();
    template<typename T>
    void push(T val) {
        //std::cout << "placing " << TypeName<T>::Get() << " into bufferWriter " << this << " at offset " << ((char*)counter-(char*)pointer) << '\n';
        // resere new space
        void* old = pointer;
        pointer = malloc((char*)counter-(char*)old+sizeof(T));
        std::memcpy(pointer, old, (char*)counter-(char*)old);
        counter=(void*)((char*)pointer+((char*)counter-(char*)old));
        // free old space
        free(old);
        // push new value
        *((T*)counter) = val;
        counter=(void*)((char*)counter+sizeof(T));
    }
    template<>
    void push<void*>(void* val) {
        //std::cout << "placing void* into bufferWriter " << this << " at offset " << ((char*)counter-(char*)pointer) << '\n';
        // resere new space
        void* old = pointer;
        pointer = malloc((char*)counter-(char*)old+sizeof(void*));
        std::memcpy(pointer, old, (char*)counter-(char*)old);
        counter=(void*)((char*)pointer+((char*)counter-(char*)old));
        // free old space
        free(old);
        // push new value
        *((void**)counter) = val;
        counter=(void*)((char*)counter+sizeof(void*));
    }
    void roundToMultipleOf(int amount);
    int getSize();
};

extern unsigned int numFunctions;
extern const char* functionMangledNames[];
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

template <typename T>
void input(bufferWriter& parameters, std::string paramName, bool doRound) {
    T tmp;
    std::cout << "Please enter a " << TypeName<T>::Get() << " for the parameter \"" << paramName << "\" : ";
    std::cin >> tmp;
    if (doRound) parameters.roundToMultipleOf(sizeof(T));
    parameters.push<T>(tmp);
}
template <>
void input<int>(bufferWriter& parameters, std::string paramName, bool doRound);
template <>
void input<bool>(bufferWriter& parameters, std::string paramName, bool doRound);
template void input<char>(bufferWriter& parameters, std::string paramName, bool doRound);
template void input<short>(bufferWriter& parameters, std::string paramName, bool doRound);
//template void input<int>(bufferWriter& parameters, std::string paramName, bool doRound);
template void input<long>(bufferWriter& parameters, std::string paramName, bool doRound);
template void input<float>(bufferWriter& parameters, std::string paramName, bool doRound);
template void input<double>(bufferWriter& parameters, std::string paramName, bool doRound);
typedef void(*inputFT)(bufferWriter&, std::string, bool);
extern std::map<std::string, inputFT> inputFunctions;
bool isInputableType(std::string type);
void inputType(std::string type, bufferWriter& parameters, std::vector<bufferWriter*>& storage, std::string paramName, bool doRound);

template <typename T>
void print(void* ptr, std::ostream& o) {
    o << (*(T*)ptr);
}
template <>
void print<bool>(void* ptr, std::ostream& o);
template <>
void print<char>(void* ptr, std::ostream& o);
template <>
void print<char*>(void* ptr, std::ostream& o);
template void print<short>(void* ptr, std::ostream& o);
template void print<int>(void* ptr, std::ostream& o);
template void print<long>(void* ptr, std::ostream& o);
template void print<float>(void* ptr, std::ostream& o);
template void print<double>(void* ptr, std::ostream& o);
typedef void(*printFT)(void*, std::ostream&);
extern std::map<std::string, printFT> printFunctions;
void printType(std::string type, void* ptr, std::ostream& o = std::cout);

extern std::map<std::string, unsigned int> typeByteLengths;
unsigned int getTypeByteLength(std::string type);

#endif // __CONTROLLER