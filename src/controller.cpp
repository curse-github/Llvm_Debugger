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

extern int numFunctions;
extern const char* functionNames[];
extern int functionParamCounts[];
extern const char** functionParamNames[];
extern const char** functionParamTypes[];
typedef void(*wrapperFT)(void*);
extern wrapperFT functionPointers[];

int main(int argc, char** argv) {
    printf("Pick a function from the following:\n");
    for(int i = 0; i < numFunctions; i++) {
        printf("  %d: %s(", i, functionNames[i]);
        int paramCount = functionParamCounts[i];
        for(int j = 0; j < paramCount; j++)
            printf((j!=(paramCount-1))?"%s %s, ":"%s %s", functionParamTypes[i][j], functionParamNames[i][j]);
        printf(")\n");
    }
    int i;
    scanf("%d", &i);

    std::string functionName = functionNames[i];
    // printf("You chose the function \"%s\".\n", functionName.c_str());
    int paramCount = functionParamCounts[i];
    // printf("\"%s\" has %d parameters.\n", functionName.c_str(), paramCount);
    bufferWriter parameters;
    for(int j = 0; j < paramCount; j++) {
        const char* paramType = functionParamTypes[i][j];
        if (strcmp(paramType, "bool") == 0) {
            char tmp[7] = "      ";
            while (true) {
                printf("Please enter a bool for the parameter \"%s\" : ", functionParamNames[i][j]);
                scanf("%6s", tmp);
                if (strcmp(tmp, "true") == 0) {
                    parameters.push<bool>(true);
                    break;
                } else if (strcmp(tmp, "false") == 0) {
                    parameters.push<bool>(false);
                    break;
                } else
                    printf("Invalid value try again.\n");
            }
        } else if (strcmp(paramType, "char") == 0) {
            char tmp;
            printf("Please enter a char for the parameter \"%s\" : ", functionParamNames[i][j]);
            scanf(" %c", &tmp);
            parameters.push<char>(tmp);
        } else if (strcmp(paramType, "int") == 0) {
            int tmp;
            printf("Please enter an int for the parameter \"%s\" : ", functionParamNames[i][j]);
            scanf("%d", &tmp);
            parameters.push<int>(tmp);
        } else if (strcmp(paramType, "float") == 0) {
            float tmp;
            printf("Please enter a float for the parameter \"%s\" : ", functionParamNames[i][j]);
            scanf("%f", &tmp);
            parameters.push<float>(tmp);
        } else if (strcmp(paramType, "double") == 0) {
            double tmp;
            printf("Please enter a double for the parameter \"%s\" : ", functionParamNames[i][j]);
            scanf("%lf", &tmp);
            parameters.push<double>(tmp);
        }
    }
    functionPointers[i](parameters.pointer);
    return 0;
}