Feb 17th 2026<br>
- Created test of “controller” c++ code which can call external functions based 1) on pointers to wrapper functions taking a bit buffer, and 2) arrays of data explaining the names of the external function’s name, arguments names, and argument types.

Feb 20th 2026<br>
- Created test of LLVM transformation pass which adds in the previously mentioned arrays of data about code procedural, rather than manually, and then compiles along with the “controller” created before.

Mar 20th<br>
- First working concept. Librarification now adds bit buffer wrapper functions and passes them into the “controller” c++ code to be called based on input from the user.

Apr 16th<br>
- Added source code for “coreutils” to project, containing common linux utilities like cat, ls, mkdir, and chown. utilized project WLLVM (whole-program LLVM) to help reverse the output executable file back into bytecode, which can then be turned into LLVM assembly code and processed by the librarification pass.

Apr 17th<br>
- Cleaned up codebase, removed codeutils code from being tracked in repository and instead added scripts to download for future users and added related filed to “.gitignore” file.

Apr 26th<br>
- Now prints the output return value from the functions chosen to be called by the user. Improved type determination by searching for types of pointer, (due to exact pointer types not being defined in LLVM). This is done by searching in the uses of the pointer for things like dereferencing the pointer, or storing a value in it, and then determining the type based on that.

May 6th<br>
- Improved pointer determination by evaluating more types of instructions for hints, improved possible crashes caused by infinite loops

Wasn’t working on this project from May 7th → July 15

Jul 16th<br>
- Began adding support for LLVM structs and arrays into librarification code, created struct definition data into output code.

Jul 17th<br>
- Added support for structs, arrays, and pointers into the controller code, taking in the user input

Jul 22nd<br>
- Improved pointer determination by checking other half of push instructions and also detects function pointers based on function calls.

Jul 24th and Jul 25th<br>
- Added debugger LLVM pass which adds log instructions before and after existing call instructions, which send debug data about the inputs and outputs of the function to external functions

Jul 28th, Jul 29th, and Jul 30th<br>
- Testing and fixing bugs with debugger code, including occasional crashes and segmentation faults.

Aug 6th, Aug 7th<br>
- Implemented clang plugin which determines the original types from the C or C++ source and stores them in a file to be read in by the llvm plugin
- Also generally improved handling of functions within namespaces or classes, in part due to the clang plugin

Aug 11th<br>
- split clang plugin into seperate parts which can be run seperately in order to run on multiple source files.

Aug 12th<br>
- finally supports overloaded functions
- fixed several issues in cases of nested structs

Aug 15th<br>
- added actual support for unions within the code and printing of them in debug mode

Aug 20th<br>
- added support for inputting unions and enums in the librarify mode for the inputting of parameters