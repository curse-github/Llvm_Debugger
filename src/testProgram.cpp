#include <iostream>

void foo(int a, double b, bool c, char d, float e) {
    printf("a: %d\nb: %.3lf\nc: %s\nd: '%c'\ne: %.3f", a, b, c?"true":"false", d, e);
}
void bar(int a, double b, bool c, char d, float e) {
    printf("a: %d\nb: %.3lf\nc: %s\nd: '%c'\ne: %.3f", a, b, c?"true":"false", d, e);
}