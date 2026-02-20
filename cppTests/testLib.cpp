#include <iostream>
#include "testLib.h"

void foo(int a, float b) {
    printf("a: %d\nb: %.3f\n", a, b);
}
void foo_wrapper(void* buffer) {
    int a = *(int*)buffer;
    buffer = (void*)((char*)buffer+sizeof(int));
    float b = *(float*)buffer;
    foo(a, b);
}

void bar(int a, bool b, char c, float d) {
    printf("a: %d\nb: %s\nc: '%c'\nd: %.3f\n", a, b?"true":"false", c, d);
}
void bar_wrapper(void* buffer) {
    int a = *(int*)buffer;
    buffer = (void*)((char*)buffer+sizeof(int));
    bool b = *(bool*)buffer;
    buffer = (void*)((char*)buffer+sizeof(bool));
    char c = *(char*)buffer;
    buffer = (void*)((char*)buffer+sizeof(char));
    float d = *(float*)buffer;
    bar(a, b, c, d);
}

#define NUM 2
int numFunctions = NUM;
const char* functionNames[NUM] = {
    "foo", "bar"
};
int functionParamCounts[NUM] = {
    2, 4
};
const char* foo_paramNames[2] = { "a", "b" };
const char* bar_paramNames[4] = { "a", "b", "c", "d" };
const char** functionParamNames[NUM] = {
    foo_paramNames, bar_paramNames
};
const char* foo_paramTypes[2] = { "int", "float" };
const char* bar_paramTypes[4] = { "int", "bool", "char", "float" };
const char** functionParamTypes[NUM] = {
    foo_paramTypes, bar_paramTypes
};
wrapperFT functionPointers[NUM] = {
    &foo_wrapper, &bar_wrapper
};

