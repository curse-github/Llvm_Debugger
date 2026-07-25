#include <iostream>

void foo1(int a, double b, bool c, char d, float e);
typedef struct {
    int a;
    double b;
    bool c;
    char d;
    float e;
} testStruct;
void foo2(testStruct g);
void bar1(float a, float b, float c, float d, float e, float f);
void bar2(float abcdef[6]);

int main(int argc, char** argv) {
    foo2({1, 2.34, false, '6', 7.8});
    return 15;
}

void foo1(int a, double b, bool c, char d, float e) {
    printf("a: %d\nb: %.3lf\nc: %s\nd: '%c'\ne: %.3f\n", a, b, c?"true":"false", d, e);
}
void foo2(testStruct g) {
    foo1(g.a, g.b, g.c, g.d, g.e);
}

void bar1(float a, float b, float c, float d, float e, float f) {
    printf("a: %.3lf\nb: %.3lf\nc: %.3lf\nd: %.3lf\ne: %.3lf\nf: %.3lf\n", a, b, c, d, e, f);
}
void bar2(float abcdef[6]) {
    bar1(abcdef[0], abcdef[1], abcdef[2], abcdef[3], abcdef[4], abcdef[5]);
}