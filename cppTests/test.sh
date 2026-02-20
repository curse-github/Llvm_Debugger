clang++ ./testLib.cpp -c -o ./testLib.o
ar rcs ./testLib.a ./testLib.o
clang++ ./testController.cpp ./testLib.a -o ./test.out
./test.out
rm ./testLib.o
rm ./testLib.a
rm test.out