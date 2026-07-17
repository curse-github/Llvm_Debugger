#include <iostream>
void foo(int a, double b, bool c, char d, float e) {
    printf("a: %d\nb: %.3lf\nc: %s\nd: '%c'\ne: %.3f\n", a, b, c?"true":"false", d, e);
}
typedef struct {
    int a;
    double b;
    bool c;
    char d;
    float e;
} testStr;
void bar(testStr g) {
    foo(g.a, g.b, g.c, g.d, g.e);
}