## Instructions:
Command to run either librarify or the debugger are as follows <br>
`make librarify TARGET="ls"`<br>
`make librarify TARGET="cat"`<br>
`make librarify TARGET="testProgram"`<br>
`make debugger TARGET="ls"`<br>
`make debugger TARGET="cat"`<br>
`make debugger TARGET="testProgram"`<br>
To add more usable targets, add instructions to the makefile file for making "./tmp/example.ll".<br>
This gives the the functionality to run:<br>
`make librarify TARGET="example"` or <br>
`make debugger TARGET="example"`<br>
<br><br>
## Writeup:
Explanation of project:
The “librarification” part of this project consists of two parts, a controller, and a modified version of the original program being analysed These two parts cover the two main portions of the code, however both portions have other challenges not covered in the basic definition of the process.<br>
The first is a controller program which receives some basic information of the structure of original code as well as pointers to wrappers of the functions. Where the wrappers of the original functions are functions which can take in a byte buffer of the parameters intended to be received by the function and then call the original function appropriately and proxy its return value. Upon running, the controller receives input from the user on the intended function to be run, gets input from the user on the parameters of that function in the correct types, packs that data into a byte buffer, calls the wrapper function, and finally outputs to the user the output of the function.<br>
The second part of the “librarification” is creating the modified version of the original program, which is done programmatically. To begin the modification it is immensely helpful to get all of the code into a single file, which in our case is done by using a tool called WLLVM or “whole-program LLVM”. Using this tool is done by compiling the program normally but using a somewhat modified compiler, and the decompiling it using the extra data obtained by using the modified compiler, into one very large LLVM assembly file. Once all of the code is in one LLVM file, you can perform a LLVM “transformation pass” which can modify and add both function code and global variables. Our code, written in c++, to perform this transformation pass first, reads the original code, getting things like function names, parameters, return values, and puts those into global variables under specific names which can be searched for by the controller later. Next, for each function it creates a wrapper function taking in a single un-typed pointer value, assumed to be a byte buffer of the correct data, and unpacks variables into the corresponding types and then calls its corresponding original function, while also proxying its return value.<br>
The first issue encountered was in the transformation pass done directly on the original code, which is, in LLVM pointers don’t have any inherit types. Yet of course we can’t simply not use pointers of any kind, so what’s the solution to this? We found that by reading the actual source code and how the pointer is used, it can sometimes be enough to determine if it is a simple pointer to an integral type. More specifically, the instructions that can be useful in determining pointer type are the following, load, store, get element pointer, and calling functions. The load and store instructions are useful fairly obviously, because they will directly state the type of the variable contained within the pointer. The get element pointer instruction and calling a function are somewhat more nuanced however. These are helpful because they will point you to another value which is guaranteed to have the same type, either within the existing function or in the other function being called. These techniques together can determine many different types of pointers within the program, though there is more work that can be done.<br>
The next problem, although admittedly easier, is inputting pointers, arrays, and structs into the byte buffers sent to the buffer functions. Additionally, in the case of structs, data within the structs may have strange alignment depending on the type. This is a small challenge since it requires extra storage to store data, and then what's actually utilized as an input is the pointer to that other data. So to fix this issue, we have a vector of the same byte buffers used as inputs for wrapper functions, which can be created as storage is needed. So when an array or pointer is needed, it creates a storage, stores the either single value or multiple consecutive values, and in the case of structs, is careful to align the values correctly. Finally, what is actually sent as the parameter to the original function is the pointer to that storage.<br>
## Timeline:
<br><br>
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
