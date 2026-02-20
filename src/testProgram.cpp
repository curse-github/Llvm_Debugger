#include <iostream>

void bar(int a, bool b, char c, float d) {
    printf("a: %d\nb: %s\nc: '%c'\nd: %.3f\n", a, b?"true":"false", c, d);
}