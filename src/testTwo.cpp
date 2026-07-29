#include <iostream>

namespace test {
    void testFunction() {
        return;
    }
}

class testClass {
    private:
    int var;
    public:
    testClass() {
        var = 0;
    }
    testClass(const testClass& copy) {
        var = copy.var;
    }
    testClass(testClass&& move) {
        var = move.var;}
    testClass& operator=(const testClass& copy) {
        var = copy.var;
        return *this;
    }
    testClass& operator=(testClass&& move) {
        var = move.var;
        return *this;
    }
    ~testClass() {
        var = 0;
    }

    void initialize() {
        var = 15;
    }
    int getVar() {
        return var;
    }
};

int main(int argc, char** argv) {
    test::testFunction();
    testClass tst1;
    tst1.initialize();
    testClass tst2((testClass&&)tst1);
    return tst2.getVar();
}