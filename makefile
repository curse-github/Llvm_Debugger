ifeq ($(OS),Windows_NT)
	objectExt = obj
	staticExt = lib
	dynamicExt = dll
	executableExt = exe
	dynamicArgs = -MD -lntdll -D_WIN32 -fexceptions -frtti ./out/libRIV.lib
else
	objectExt = o
	staticExt = a
	dynamicExt = so
	executableExt = out
	dynamicArgs = -fPIC
endif

includedir = $(shell llvm-config --includedir)
libs = $(shell llvm-config --ldflags --libs core support passes)

.phony : librarify stdlib mkdir clean

./tmp/testOne.ll: ./src/testOne.cpp
	@export LLVM_COMPILER=clang && ./wllvm_venv/bin/wllvm ./src/testOne.cpp -O0 -fno-discard-value-names -fno-inline -c -o ./tmp/testOne.o
	@./wllvm_venv/bin/extract-bc ./tmp/testOne.o -o ./tmp/testOne.bc
	@llvm-dis ./tmp/testOne.bc -o ./tmp/testOne.ll
./tmp/testTwo.ll: ./src/testTwo.cpp
	@export LLVM_COMPILER=clang && ./wllvm_venv/bin/wllvm ./src/testTwo.cpp -O0 -fno-discard-value-names -fno-inline -c -o ./tmp/testTwo.o
	@./wllvm_venv/bin/extract-bc ./tmp/testTwo.o -o ./tmp/testTwo.bc
	@llvm-dis ./tmp/testTwo.bc -o ./tmp/testTwo.ll
./tmp/ls.ll:
	@./wllvm_venv/bin/extract-bc ./coreutils/src/ls -o ./tmp/ls.bc
	@llvm-dis ./tmp/ls.bc -o ./tmp/ls.ll
./tmp/cat.ll:
	@./wllvm_venv/bin/extract-bc ./coreutils/src/cat -o ./tmp/cat.bc
	@llvm-dis ./tmp/cat.bc -o ./tmp/cat.ll

librarify: mkdir ./tmp/$(TARGET).ll libLibrarify.$(dynamicExt)
	@-echo
	@-echo running librarify.$(dynamicExt) pass on $(TARGET).ll
	@opt -load-pass-plugin ./out/libLibrarify.$(dynamicExt) -passes librarify ./tmp/$(TARGET).ll -S -o ./tmp/output_from_librarify.ll
	@clang++ ./tmp/output_from_librarify.ll -c -o ./tmp/output.o
	@ar rcs ./out/output.a ./tmp/output.o
	@-echo
	@-echo compiling librarifyController.$(executableExt)
	@clang++ -I./include ./src/librarifyController.cpp ./src/controllerLib.cpp ./out/output.a -lcap -o ./out/librarifyController.$(executableExt)
	@-echo running librarifyController.$(executableExt)
	@-echo
	@./out/librarifyController.$(executableExt)

debugger: mkdir ./tmp/$(TARGET).ll libLibrarify.$(dynamicExt) libDebugger.$(dynamicExt)
	@-echo
	@-echo running librarify.$(dynamicExt) pass on $(TARGET).ll
	@opt -load-pass-plugin ./out/libLibrarify.$(dynamicExt) -passes librarify ./tmp/$(TARGET).ll -S -o ./tmp/output_from_librarify.ll
	@-echo running debugger.$(dynamicExt) pass on $(TARGET).ll
	@opt -load-pass-plugin ./out/libDebugger.$(dynamicExt) -passes debugger ./tmp/output_from_librarify.ll -S -o ./tmp/output_from_debugger.ll
	@clang++ ./tmp/output_from_debugger.ll -c -o ./tmp/output.o
	@ar rcs ./out/output.a ./tmp/output.o
	@-echo
	@-echo compiling debuggerController.$(executableExt)
	@clang++ -I./include ./src/debuggerController.cpp ./src/controllerLib.cpp ./out/output.a -lcap -o ./out/debuggerController.$(executableExt)
	@-echo running debuggerController.$(executableExt)
	@-echo
	@./out/debuggerController.$(executableExt)

stdlib: mkdir ./lib/cppStdLib.cpp ./lib/llvmStdLibWin.ll ./lib/llvmStdLibLin.ll
	@-echo building std lib
	@clang++ -Werror -Wall -O3 ./lib/cppStdLib.cpp -c -o ./tmp/cppStdLib.$(objectExt)
ifeq ($(OS),Windows_NT)
	@clang++ -Werror -Wall -O3 ./lib/llvmStdLibWin.ll -c -o ./tmp/llvmStdLib.$(objectExt)
else
	@clang++ -Werror -Wall -O3 ./lib/llvmStdLibLin.ll -c -o ./tmp/llvmStdLib.$(objectExt)
endif
	@ar rcs ./out/libStd.$(staticExt) ./tmp/cppStdLib.$(objectExt) ./tmp/llvmStdLib.$(objectExt)
	@-echo finished building std lib

libLibrarify.$(dynamicExt): mkdir ./src/Librarify.cpp ./src/llvmHelpers.cpp
	@-echo building libLibrarify.$(dynamicExt)
	@clang++ $(dynamicArgs) -Werror -Wall -Wno-unused-command-line-argument -Wno-deprecated-declarations -fdeclspec -std=c++23 -O3 -I$(includedir) -I./include ./src/Librarify.cpp ./src/llvmHelpers.cpp $(libs) -shared -o ./out/libLibrarify.$(dynamicExt)
	@-echo finished building libLibrarify.$(dynamicExt)
libDebugger.$(dynamicExt): mkdir ./src/Debugger.cpp ./src/llvmHelpers.cpp
	@-echo building libDebugger.$(dynamicExt)
	@clang++ $(dynamicArgs) -Werror -Wall -Wno-unused-command-line-argument -Wno-deprecated-declarations -fdeclspec -std=c++23 -O3 -I$(includedir) -I./include ./src/Debugger.cpp ./src/llvmHelpers.cpp $(libs) -shared -o ./out/libDebugger.$(dynamicExt)
	@-echo finished building libDebugger.$(dynamicExt)

mkdir:
ifeq ($(OS),Windows_NT)
	@-rmdir /s /q out
	@-rmdir /s /q tmp
else
	@-rm -rf out
	@-rm -rf tmp
endif
	@mkdir out
	@mkdir tmp

clean:
	@-rm -R out
	@-rm -R tmp