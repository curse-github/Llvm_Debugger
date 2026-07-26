#include <iostream>

typedef struct {
    int a;
    double b;
    bool c;
    char d;
    float e;
} testStruct;
int foo1(int a, double b, bool c, char d, float e) {
    return c?(a+d):((int)b+e);
}
int foo2(testStruct g) {
    return foo1(g.a, g.b, g.c, g.d, g.e);
}

float bar1(float a, float b, float c, float d, float e, float f) {
    return a+b-c+d-e+f;
}
float bar2(float abcdef[6]) {
    return bar1(abcdef[0], abcdef[1], abcdef[2], abcdef[3], abcdef[4], abcdef[5]);
}

int main(int argc, char** argv) {
    for(int i = 0; i < argc; i++) {
        std::cout << argv[i] << '\n';
    }
    foo2({1, 2.34, false, '6', 7.8});
    float tmp[6] = { 1.2, 3.4, 5.6, 7.8, 9.10, 11.12 };
    bar2(tmp);
    return 15;
}