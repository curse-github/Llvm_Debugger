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

./tmp/input_for_librarify.ll:
	@-echo creating input_for_librarify.bc
	@./wllvm_venv/bin/clang++ ./src/testProgram.cpp -c -O3 -fno-discard-value-names -fno-inline -o ./tmp/input_for_librarify.o
./tmp/ls.ll:
	@./wllvm_venv/bin/extract-bc ./coreutils/src/ls -o ./tmp/ls.bc
	@llvm-dis ./tmp/ls.bc -o ./tmp/ls.ll
./tmp/cat.ll:
	@./wllvm_venv/bin/extract-bc ./coreutils/src/ls -o ./tmp/cat.bc
	@llvm-dis ./tmp/cat.bc -o ./tmp/cat.ll

test: mkdir ./tmp/ls.ll libLibrarify.$(dynamicExt)
	@-echo running libLibrarify.$(dynamicExt) plugin on ls.ll
	@opt -load-pass-plugin ./out/libLibrarify.$(dynamicExt) -passes librarify ./tmp/ls.ll -S -o ./tmp/output_from_librarify.ll
	@clang++ ./tmp/output_from_librarify.ll -c -o ./tmp/output.o
	@ar rcs ./out/output.a ./tmp/output.o
	@-echo compiling controller.$(executableExt)
	@clang++ ./src/controller.cpp ./out/output.a -lcap -o ./out/controller.$(executableExt)
	@-echo running controller.$(executableExt)
	@#./out/controller.$(executableExt)
.phony : test

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
.phony : stdlib

libLibrarify.$(dynamicExt): mkdir ./src/Librarify.cpp ./src/llvmHelpers.cpp
	@-echo building libLibrarify.$(dynamicExt)
	@clang++ $(dynamicArgs) -Werror -Wall -Wno-unused-command-line-argument -Wno-deprecated-declarations -fdeclspec -std=c++23 -O3 -I$(includedir) -I./include ./src/Librarify.cpp ./src/llvmHelpers.cpp $(libs) -shared -o ./out/libLibrarify.$(dynamicExt)
	@-echo finished building libLibrarify.$(dynamicExt)

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
.phony : mkdir

clean:
	@-rm -R out
	@-rm -R tmp
.phony : clean