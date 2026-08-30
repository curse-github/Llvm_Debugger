#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
void printVaList(int numArgs, ...) {
    va_list list;
    va_start(list, numArgs);
    for (int i = 0; i < numArgs; i++)
        printf("%.3f ", va_arg(list, double));
    printf("\n");
    va_end(list);
}
int main(void) {
    printVaList(1, 1.2);
    printVaList(3, 7.8, 9.01, 11.12);
}