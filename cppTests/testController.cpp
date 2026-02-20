#include <cstdlib>
#include <cstring>
#include <iostream>
using std::memcpy;
using std::strcmp;
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
        memcpy(pointer,old,(char*)counter-(char*)old);
        counter=(void*)((char*)counter-(char*)old+(char*)pointer);
        *(T*)counter = val;
        counter=(void*)((char*)counter+sizeof(T));
    }
};

#include "testLib.h"

int main(int argc, char** argv) {
    printf("Pick a function from the following:\n");
    for(int i = 0; i < numFunctions; i++) {
        printf("  %d: %s\n", i, functionNames[i]);
    }
    int i = 0;
    scanf("%d", &i);
    printf("You chose the function \"%s\".\n", functionNames[i]);
    bufferWriter parameters;
    for(int j = 0; j < functionParamCounts[i]; j++) {
        const char* paramType = functionParamTypes[i][j];
        printf("Please enter a(n) %s for the parameter \"%s\" : ", paramType, functionParamNames[i][j]);
        if (strcmp(paramType, "bool") == 0) {
            char tmp[7] = "      ";
            scanf("%6s", tmp);
            if (strcmp(tmp,"true") == 0)
                parameters.push<bool>(true);
            else if (strcmp(tmp,"false") == 0)
                parameters.push<bool>(false);
            else {
                printf("ERROR\n");
                return 1;
            }
        } else if (strcmp(paramType, "char") == 0) {
            char tmp;
            scanf(" %c", &tmp);
            parameters.push<char>(tmp);
        } else if (strcmp(paramType, "int") == 0) {
            int tmp;
            scanf("%d", &tmp);
            parameters.push<int>(tmp);
        } else if (strcmp(paramType, "float") == 0) {
            float tmp;
            scanf("%f", &tmp);
            parameters.push<float>(tmp);
        }
    }
    functionPointers[i](parameters.pointer);
    return 0;
}