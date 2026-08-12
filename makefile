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

clang-plugin-args = -Xclang -load -Xclang ./out/libClangPlugin.so -Xclang -add-plugin -Xclang
compile-args = -O0 -fno-discard-value-names -fno-inline

.phony : librarify debugger stdlib mkdir clean

./tmp/testOne.ll: ./src/testOne.cpp ./out/libClangPlugin.so
	@export LLVM_COMPILER=clang && ./wllvm_venv/bin/wllvm ./src/testOne.cpp $(compile-args) -c -o ./tmp/testOne.o
	@./wllvm_venv/bin/extract-bc ./tmp/testOne.o -o ./tmp/testOne.bc
	@llvm-dis ./tmp/testOne.bc -o ./tmp/testOne.ll
	@clang $(clang-plugin-args) $(compile-args) -fsyntax-only ./src/testOne.cpp
./tmp/testTwo.ll: ./src/testTwo.cpp ./out/libClangPlugin.so
	@export LLVM_COMPILER=clang && ./wllvm_venv/bin/wllvm ./src/testTwo.cpp $(compile-args) -c -o ./tmp/testTwo.o
	@./wllvm_venv/bin/extract-bc ./tmp/testTwo.o -o ./tmp/testTwo.bc
	@llvm-dis ./tmp/testTwo.bc -o ./tmp/testTwo.ll
	@clang $(clang-plugin-args) $(compile-args) -fsyntax-only ./src/testTwo.cpp
./tmp/testThree.ll: ./src/testThree.c ./out/libClangPlugin.so
	@export LLVM_COMPILER=clang && ./wllvm_venv/bin/wllvm ./src/testThree.c $(compile-args) -c -o ./tmp/testThree.o
	@./wllvm_venv/bin/extract-bc ./tmp/testThree.o -o ./tmp/testThree.bc
	@llvm-dis ./tmp/testThree.bc -o ./tmp/testThree.ll
	@clang $(clang-plugin-args) $(compile-args) -fsyntax-only ./src/testThree.c
./tmp/ls.ll: ./coreutils/src/ls ./coreutils/src/ls.c ./out/libClangPlugin.so
	@./wllvm_venv/bin/extract-bc ./coreutils/src/ls -o ./tmp/ls.bc
	@llvm-dis ./tmp/ls.bc -o ./tmp/ls.ll
	@clang $(clang-plugin-args) $(compile-args) -fsyntax-only -I./coreutils/lib ./coreutils/src/ls.c
./tmp/cat.ll:./coreutils/src/cat ./coreutils/src/cat.c ./out/libClangPlugin.so
	@./wllvm_venv/bin/extract-bc ./coreutils/src/cat -o ./tmp/cat.bc
	@llvm-dis ./tmp/cat.bc -o ./tmp/cat.ll
	@clang $(clang-plugin-args) save-typedefs $(compile-args) -fsyntax-only -I./coreutils/lib ./coreutils/lib/quotearg.c ./coreutils/src/cat.c
	@clang $(clang-plugin-args) save-func-parms $(compile-args) -fsyntax-only -I./coreutils/lib ./coreutils/lib/quotearg.c ./coreutils/src/cat.c

librarify: mkdir ./tmp/$(TARGET).ll ./out/libLlvmPass.$(dynamicExt)
	@-echo
	@-echo running librarify.$(dynamicExt) pass on $(TARGET).ll
	@opt -load-pass-plugin ./out/libLlvmPass.$(dynamicExt) -passes librarify ./tmp/$(TARGET).ll -S -o ./tmp/library_$(TARGET).ll
	@clang++ ./tmp/library_$(TARGET).ll -c -o ./tmp/library_$(TARGET).o
	@ar rcs ./out/$(TARGET).$(staticExt) ./tmp/library_$(TARGET).o
	@-echo
	@-echo compiling librarifyController.$(executableExt)
	@clang++ -I./include ./src/controllers/librarifyController.cpp ./src/controllers/controllerLib.cpp ./out/$(TARGET).$(staticExt) -o ./out/librarifyController.$(executableExt)
	@-echo running librarifyController.$(executableExt)
	@-echo
	@./out/librarifyController.$(executableExt)

debugger: mkdir ./tmp/$(TARGET).ll ./out/libLlvmPass.$(dynamicExt)
	@-echo
	@-echo running librarify and logger pass on $(TARGET).ll
	@opt -load-pass-plugin ./out/libLlvmPass.$(dynamicExt) -passes librarify,logger ./tmp/$(TARGET).ll -S -o ./tmp/library_$(TARGET).ll
	@clang++ ./tmp/library_$(TARGET).ll -c -o ./tmp/library_$(TARGET).o
	@ar rcs ./out/$(TARGET).$(staticExt) ./tmp/library_$(TARGET).o
	@-echo
	@-echo compiling debuggerController.$(executableExt)
	@clang++ -I./include ./src/controllers/debuggerController.cpp ./src/controllers/controllerLib.cpp ./out/$(TARGET).$(staticExt) -o ./out/debuggerController.$(executableExt)
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

./out/libLlvmPass.$(dynamicExt): ./src/llvm_pass/Logger.cpp ./src/llvm_pass/Librarify.cpp ./src/llvm_pass/llvmHelpers.cpp
	@-echo building libLlvmPass.$(dynamicExt)
	@clang++ $(dynamicArgs) -Werror -Wall -Wno-unused-command-line-argument -Wno-deprecated-declarations -fdeclspec -std=c++23 -O3 -I$(includedir) -I./include ./src/llvm_pass/getPassInfo.cpp ./src/llvm_pass/Logger.cpp ./src/llvm_pass/Librarify.cpp ./src/llvm_pass/llvmHelpers.cpp $(libs) -shared -o ./out/libLlvmPass.$(dynamicExt)
	@-echo finished building libLlvmPass.$(dynamicExt)
./out/libClangPlugin.$(dynamicExt): ./src/clang_plugin/saveFuncParms.cpp ./src/clang_plugin/saveTypedefs.cpp ./src/clang_plugin/clangHelpers.cpp
	@-echo building libClangPlugin.$(dynamicExt)
	@clang++ $(dynamicArgs) -Werror -Wall -Wno-unused-command-line-argument -Wno-deprecated-declarations -fdeclspec -std=c++23 -O3 -I$(includedir) -I./include ./src/clang_plugin/saveFuncParms.cpp ./src/clang_plugin/saveTypedefs.cpp ./src/clang_plugin/clangHelpers.cpp $(libs) -shared -o ./out/libClangPlugin.$(dynamicExt)
	@-echo finished building libClangPlugin.$(dynamicExt)

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