#include <iostream>
void foo1(int a, double b, bool c, char d, float e) {
    printf("a: %d\nb: %.3lf\nc: %s\nd: '%c'\ne: %.3f\n", a, b, c?"true":"false", d, e);
}
typedef struct {
    int a;
    double b;
    bool c;
    char d;
    float e;
} testStruct;
void foo2(testStruct g) {
    foo1(g.a, g.b, g.c, g.d, g.e);
}
void foo3(int a, double b, bool c, char d, float e) {
    foo2(testStruct{a,b,c,d,e});
}

void bar1(float a, float b, float c, float d, float e, float f) {
    printf("a: %.3lf\nb: %.3lf\nc: %.3lf\nd: %.3lf\ne: %.3lf\nf: %.3lf\n", a, b, c, d, e, f);
}
typedef struct {
    float abcdef[6];
} tmp;
void bar2(tmp g) {
    bar1(g.abcdef[0], g.abcdef[1], g.abcdef[2], g.abcdef[3], g.abcdef[4], g.abcdef[5]);
}