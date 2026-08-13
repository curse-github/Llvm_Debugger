#include <iostream>

int foo(int a, double b, bool c, char d, float e) {
    return c?(a+d):((int)b+e);
}
typedef struct {
    int a;
    double b;
    bool c;
    char d;
    float e;
} fooArgs;
int foo(fooArgs g) {
    return foo(g.a, g.b, g.c, g.d, g.e);
}
struct testStruct {
    char buf;
    int buff;
    char bufff;
    fooArgs g;
    testStruct() {
        buf = '0';
        buff = 1;
        bufff = '2';
    }
};
int foo(testStruct h) {
    return foo(h.g);
}

float bar(float a, float b, float c, float d, float e, float f) {
    return a+b-c+d-e+f;
}
float bar(float abcdef[6]) {
    return bar(abcdef[0], abcdef[1], abcdef[2], abcdef[3], abcdef[4], abcdef[5]);
}

typedef struct {
    bool isFloat;
    union {
        int type1;
        float type2;
        char buffer[15];
    } value;
} testUnion;
void createUnion(testUnion* result, int value) {
    result->isFloat = false;
    result->value.type1 = value;
    return;
}
void createUnion(testUnion* result, float value) {
    result->isFloat = true;
    result->value.type2 = value;
    return;
}

int main(int argc, char** argv) {
    testStruct tmp1{};
    tmp1.g = {1, 2.34, false, '6', 7.8f};
    foo(tmp1);
    float tmp2[6] = { 1.2, 3.4, 5.6, 7.8, 9.10, 11.12 };
    bar(tmp2);
    testUnion output;
    createUnion(&output, 12);
    createUnion(&output, 2.0f);
    return 15;
}