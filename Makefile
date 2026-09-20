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

LLVM_BIN := /usr/lib/llvm-22/bin
export PATH := $(LLVM_BIN):$(PATH)

includedir = $(shell llvm-config --includedir)
libs = $(shell llvm-config --ldflags --libs core support passes)

clang-plugin-args = -Xclang -load -Xclang ./out/libClangPlugin.so -Xclang -add-plugin -Xclang
compile-args = -g -O0 -fno-inline -Wall -Wextra -Wno-implicit-function-declaration -fno-discard-value-names -Wno-c23-extensions -Wno-sign-compare -Wno-tautological-constant-out-of-range-compare
ifeq ($(TARGET),sort)
additional-libs = -lcrypto
else
additional-libs = 
endif

.phony : default librarify debugger stdlib mkdir clean

default: ./tmp/$(TARGET).ll
	@-echo

./tmp/testOne.ll: ./src/testOne.cpp ./out/libClangPlugin.$(dynamicExt)
	@export LLVM_COMPILER=clang && ./wllvm_venv/bin/wllvm ./src/testOne.cpp $(compile-args) -c -o ./tmp/testOne.$(objectExt)
	@./wllvm_venv/bin/extract-bc ./tmp/testOne.$(objectExt) -o ./tmp/testOne.bc
	@llvm-dis ./tmp/testOne.bc -o ./tmp/testOne.ll
	@-echo running clang plugin on testOne.cpp
	@clang $(clang-plugin-args) save-typedefs $(compile-args) -fsyntax-only -Wno-everything ./src/testOne.cpp
	@clang $(clang-plugin-args) save-func-parms $(compile-args) -fsyntax-only -Wno-everything ./src/testOne.cpp
./tmp/testTwo.ll: ./src/testTwo.cpp ./out/libClangPlugin.$(dynamicExt)
	@export LLVM_COMPILER=clang && ./wllvm_venv/bin/wllvm ./src/testTwo.cpp $(compile-args) -c -o ./tmp/testTwo.$(objectExt)
	@./wllvm_venv/bin/extract-bc ./tmp/testTwo.$(objectExt) -o ./tmp/testTwo.bc
	@llvm-dis ./tmp/testTwo.bc -o ./tmp/testTwo.ll
	@-echo running clang plugin on testTwo.cpp
	@clang $(clang-plugin-args) save-typedefs $(compile-args) -fsyntax-only -Wno-everything ./src/testTwo.cpp
	@clang $(clang-plugin-args) save-func-parms $(compile-args) -fsyntax-only -Wno-everything ./src/testTwo.cpp
./tmp/testThree.ll: ./src/testThree.c ./out/libClangPlugin.$(dynamicExt)
	@export LLVM_COMPILER=clang && ./wllvm_venv/bin/wllvm ./src/testThree.c $(compile-args) -c -o ./tmp/testThree.$(objectExt)
	@./wllvm_venv/bin/extract-bc ./tmp/testThree.$(objectExt) -o ./tmp/testThree.bc
	@llvm-dis ./tmp/testThree.bc -o ./tmp/testThree.ll
	@-echo running clang plugin on testThree.c
	@clang $(clang-plugin-args) save-typedefs $(compile-args) -fsyntax-only -Wno-everything ./src/testThree.c
	@clang $(clang-plugin-args) save-func-parms $(compile-args) -fsyntax-only -Wno-everything ./src/testThree.c
./tmp/ls.ll: ./out/libClangPlugin.$(dynamicExt) ./coreutils/src/$(TARGET) ./coreutils/src/$(TARGET).c
	@./wllvm_venv/bin/extract-bc ./coreutils/src/ls -o ./tmp/ls.bc
	@llvm-dis ./tmp/ls.bc -o ./tmp/ls.ll
	@-echo running clang plugin on ls.c
	@clang $(clang-plugin-args) save-typedefs $(compile-args) -fsyntax-only -Wno-everything -I./coreutils/lib ./coreutils/src/ls.c
	@clang $(clang-plugin-args) save-func-parms $(compile-args) -fsyntax-only -Wno-everything -I./coreutils/lib ./coreutils/src/ls.c
./tmp/cat.ll: ./out/libClangPlugin.$(dynamicExt) ./coreutils/src/$(TARGET) ./coreutils/src/$(TARGET).c
	@./wllvm_venv/bin/extract-bc ./coreutils/src/cat -o ./tmp/cat.bc
	@llvm-dis ./tmp/cat.bc -o ./tmp/cat.ll
	@-echo running clang plugin on cat.c
	@clang $(clang-plugin-args) save-typedefs $(compile-args) -fsyntax-only -Wno-everything -I./coreutils/lib ./coreutils/src/cat.c
	@clang $(clang-plugin-args) save-func-parms $(compile-args) -fsyntax-only -Wno-everything -I./coreutils/lib ./coreutils/src/cat.c
./tmp/split.ll: ./out/libClangPlugin.$(dynamicExt)
	@./wllvm_venv/bin/extract-bc ./coreutils/src/split -o ./tmp/split.bc
	@llvm-dis ./tmp/split.bc -o ./tmp/split.ll
	@-echo running clang plugin on split.c
	@clang $(clang-plugin-args) save-typedefs $(compile-args) -fsyntax-only -Wno-everything -I./coreutils/lib ./coreutils/src/split.c
	@clang $(clang-plugin-args) save-func-parms $(compile-args) -fsyntax-only -Wno-everything -I./coreutils/lib ./coreutils/src/split.c
./tmp/sort.ll: ./out/libClangPlugin.$(dynamicExt) ./coreutils/src/$(TARGET) ./coreutils/src/$(TARGET).c
	@./wllvm_venv/bin/extract-bc ./coreutils/src/sort -o ./tmp/sort.bc
	@llvm-dis ./tmp/sort.bc -o ./tmp/sort.ll
	@-echo running clang plugin on sort.c
	@clang $(clang-plugin-args) save-typedefs $(compile-args) -fsyntax-only -Wno-everything -I./coreutils/lib ./coreutils/src/sort.c
	@clang $(clang-plugin-args) save-func-parms $(compile-args) -fsyntax-only -Wno-everything -I./coreutils/lib ./coreutils/src/sort.c

librarify: mkdir ./tmp/$(TARGET).ll ./out/libLlvmPass.$(dynamicExt) ./tmp/controllerLib.$(objectExt) ./src/controllers/librarifyController.cpp
	@-echo running librarify pass on $(TARGET).ll
	@opt -load-pass-plugin ./out/libLlvmPass.$(dynamicExt) -passes librarify ./tmp/$(TARGET).ll -S -o ./tmp/library_$(TARGET).ll
	@clang++ ./tmp/library_$(TARGET).ll $(additional-libs) -c -o ./tmp/library_$(TARGET).$(objectExt)
	@ar rcs ./out/$(TARGET).$(staticExt) ./tmp/library_$(TARGET).$(objectExt)
	@clang++ -I./include ./src/controllers/librarifyController.cpp -c -o ./tmp/controller.$(objectExt)
	@clang++ ./tmp/controller.$(objectExt) ./tmp/controllerLib.$(objectExt) ./out/$(TARGET).$(staticExt) -lcap -o ./out/$(TARGET).$(executableExt)

debugger: mkdir ./tmp/$(TARGET).ll ./out/libLlvmPass.$(dynamicExt) ./tmp/controllerLib.$(objectExt) ./src/controllers/debuggerController.cpp
	@-echo running librarify and logger pass on $(TARGET).ll
	@opt -load-pass-plugin ./out/libLlvmPass.$(dynamicExt) -passes librarify,logger ./tmp/$(TARGET).ll -S -o ./tmp/library_$(TARGET).ll
	@clang++ ./tmp/library_$(TARGET).ll -g -c -o ./tmp/library_$(TARGET).$(objectExt)
	@ar rcs ./out/$(TARGET).$(staticExt) ./tmp/library_$(TARGET).$(objectExt)
	@clang++ -I./include ./src/controllers/debuggerController.cpp -g -c -o ./tmp/controller.$(objectExt)
	@clang++ ./tmp/controller.$(objectExt) ./tmp/controllerLib.$(objectExt) -g ./out/$(TARGET).$(staticExt) $(additional-libs) -lcap -o ./out/$(TARGET).$(executableExt)

./tmp/controllerLib.$(objectExt) : 
	@clang++ -I./include ./src/controllers/controllerLib.cpp -c -o ./tmp/controllerLib.$(objectExt)
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
	@-rm -R ./split_vulnerability/out
