#include <stdio.h>
float myPrint(float val) {
    printf("this is a test of running c code with my debugger, %.3lf\n", val);
    return val*2;
}
int main(void) {
    myPrint(1.2);
    myPrint(3.4);
    myPrint(5.6);
    myPrint(7.8);
    myPrint(9.01);
    myPrint(11.12);
}