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
.\tmp\input_for_passes.ll:
	@-echo creating input_for_passes.ll
ifeq ($(OS),Windows_NT)
	@type .\strLen.ll >> .\tmp\input_for_passes.ll
else
	@cp ./strLen.ll ./tmp/input_for_passes.ll
endif
.\tmp\input_for_librarify.ll:
	@-echo creating input_for_librarify.ll
	@clang++ ./src/testProgram.cpp -O3 -S -emit-llvm -fno-discard-value-names -fno-inline -o ./tmp/input_for_librarify.ll

testLibrarify: mkdir .\tmp\input_for_librarify.ll libLibrarify.$(dynamicExt)
	@-echo running libLibrarify.$(dynamicExt) plugin on input_for_librarify.ll
	@opt -load-pass-plugin ./out/libLibrarify.$(dynamicExt) -passes librarify ./tmp/input_for_librarify.ll -S -o ./tmp/output_from_librarify.ll
	@clang++ ./tmp/output_from_librarify.ll -c -o ./tmp/output.o
	@ar rcs ./out/output.a ./tmp/output.o
	@clang++ ./src/controller.cpp ./out/output.a -o ./out/controller.out
	@#clear
	@./out/controller.out

testSeparate: mkdir .\tmp\input_for_passes.ll stdlib libSeparate.$(dynamicExt)
	@-echo running libSeparate.$(dynamicExt) plugin on input_for_passes.ll
	@#-echo --------------------------------
	@opt -load-pass-plugin ./out/libSeparate.$(dynamicExt) -passes separate ./tmp/input_for_passes.ll -disable-output
	@#-echo --------------------------------

testAddDebugPrint: mkdir .\tmp\input_for_passes.ll stdlib libAddDebugPrint.$(dynamicExt)
	@-echo running libAddDebugPrint.$(dynamicExt) plugin on input_for_passes.ll
	@opt -load-pass-plugin ./out/libAddDebugPrint.$(dynamicExt) -passes add-debug-print ./tmp/input_for_passes.ll -S -o ./tmp/output_from_add_debug_print.ll
	@#-echo --------------------------------

	@clang++ -Werror -Wno-override-module -std=c++23 -O3 ./tmp/input_for_passes.ll ./out/libStd.$(staticExt) -o ./out/input.$(executableExt)
	@clang++ -Werror -Wno-override-module -std=c++23 -O3 ./tmp/output_from_add_debug_print.ll ./out/libStd.$(staticExt) -o ./out/output.$(executableExt)
	
	@-echo --------------------------------
	@-echo testing output.$(executableExt)
	@-./out/output.$(executableExt)
	@-echo
	@-echo testing input.$(executableExt) \"\"
	@-./out/output.$(executableExt) ""
	@-echo
	@-echo testing input.$(executableExt) \"abcdefghijklmnopqrstuvwxyz\"
	@-./out/output.$(executableExt) "abcdefghijklmnopqrstuvwxyz"
	@-echo --------------------------------
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

libAddDebugPrint.$(dynamicExt): mkdir ./src/AddDebugPrint.cpp ./src/llvmHelpers.cpp
	@-echo building libAddDebugPrint.$(dynamicExt)
	@clang++ $(dynamicArgs) -Werror -Wall -Wno-unused-command-line-argument -Wno-deprecated-declarations -fdeclspec -std=c++23 -O3 -I$(includedir) -I./include ./src/AddDebugPrint.cpp ./src/llvmHelpers.cpp $(libs) -shared -o ./out/libAddDebugPrint.$(dynamicExt)
	@-echo finished building libAddDebugPrint.$(dynamicExt)
libSeparate.$(dynamicExt): mkdir ./src/Separate.cpp
	@-echo building libSeparate.$(dynamicExt)
	@clang++ $(dynamicArgs) -Werror -Wall -Wno-unused-command-line-argument -Wno-deprecated-declarations -fdeclspec -std=c++23 -O3 -I$(includedir) -I./include ./src/Separate.cpp $(libs) -shared -o ./out/libSeparate.$(dynamicExt)
	@-echo finished building libSeparate.$(dynamicExt)
libLibrarify.$(dynamicExt): mkdir ./src/Librarify.cpp ./src/llvmHelpers.cpp
	@-echo building libLibrarify.$(dynamicExt)
	@clang++ $(dynamicArgs) -Werror -Wall -Wno-unused-command-line-argument -Wno-deprecated-declarations -fdeclspec -std=c++23 -O3 -I$(includedir) -I./include ./src/Librarify.cpp ./src/llvmHelpers.cpp $(libs) -shared -o ./out/libLibrarify.$(dynamicExt)
	@-echo finished building libLibrarify.$(dynamicExt)

mkdir:
ifeq ($(OS),Windows_NT)
	@-rmdir /s /q out
	@-rmdir /s /q tmp
	@-rmdir /s /q separated
else
	@-rm -rf out
	@-rm -rf tmp
	@-rm -rf separated
endif
	@mkdir out
	@mkdir tmp
	@mkdir separated
.phony : mkdir

clean:
	@-rm -R out
	@-rm -R tmp
	@-rm -R separated
.phony : clean