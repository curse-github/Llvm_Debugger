#include <iostream>
// testing arguments and return types
float foo(float a, float b, float c, float d, float e, float f) {
    return a+b-c+d-e+f;
}
// testing overloads and arrays
float foo(float abcdef[6]) {
    return foo(abcdef[0], abcdef[1], abcdef[2], abcdef[3], abcdef[4], abcdef[5]);
}

int bar(int a, double b, bool c, char d, float e) {
    return c?(a+d):((int)b+e);
}
typedef struct {
    int a;
    double b;
    bool c;
    char d;
    float e;
} fooArgs;
// testing inputting structs
int bar(fooArgs g) {
    return bar(g.a, g.b, g.c, g.d, g.e);
}
// testing correct struct field offsets and initializers
struct testStruct {
    char buf;// at offset 0
    int buff;// at offset 4
    char bufff;// at offset 8
    fooArgs g;// at offset 16
    testStruct() {
        buf = '0';
        buff = 1;
        bufff = '2';
    }
};
int bar(testStruct h) {
    return bar(h.g);
}


typedef union {
    int type1;
    float type2;
    char buffer[15];
} testUnion;
typedef struct {
    bool isFloat;
    testUnion value;
} unionHandler;
// testing unions and nested unions
void createUnion(unionHandler* result, int value) {
    result->isFloat = false;
    result->value.type1 = value;
    return;
}
void createUnion(unionHandler* result, float value) {
    result->isFloat = true;
    result->value.type2 = value;
    return;
}

// testing enums
enum testEnum : int {
    TEST_LOW = 10,
    TEST_MEDIUM = 5,
    TEST_HIGH = 0,
    TEST_ERROR = -1
};
void printEnum(testEnum val) {
    switch (val) {
        case TEST_LOW:
            std::cout << "LOW\n";
            break;
        case TEST_MEDIUM:
            std::cout << "MEDIUM\n";
            break;
        case TEST_HIGH:
            std::cout << "HIGH\n";
            break;
        default:
            std::cout << "ERROR\n";
            break;
    }
}

int main() {// int argc, char** argv) {
    float tmp2[6] = { 1.2, 3.4, 5.6, 7.8, 9.10, 11.12 };
    foo(tmp2);
    testStruct tmp1{};
    tmp1.g = {1, 2.34, false, '6', 7.8f};
    bar(tmp1);

    unionHandler output;
    createUnion(&output, 12);
    createUnion(&output, 2.0f);

    printEnum(TEST_HIGH);
    printEnum(TEST_LOW);
    return 15;
}