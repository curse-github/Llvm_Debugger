## General
this is run on my machine running arch linux, with llvm version 21
## Setup
the project can be setup by simply running ./config.sh if on ubuntu<br>
otherwise install the following programs, and run the ./create_coreutils.sh script<br>
python3 python3-pip make git wget
## Instructions:
Command to run either librarify or the debugger are as follows <br>
`make librarify TARGET=testOne`<br>
`make librarify TARGET=testTwo`<br>
`make librarify TARGET=testThree`<br>
`make librarify TARGET=ls`<br>
`make librarify TARGET=cat`<br>
`make debugger TARGET=testOne`<br>
`make debugger TARGET=testTwo`<br>
`make debugger TARGET=testThree`<br>
`make debugger TARGET=ls ARGS="."`<br>
`make debugger TARGET=cat ARGS="./exFile.txt"`<br>
To add more usable targets, add instructions to the makefile file for making ./tmp/"example".ll.<br>
This makefile target needs to create the file ./tmp/"example".ll, but also 
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
The next part of this project is the actual “debugger” part, another LLVM pass run directly after librarify. The goal of this portion of the project is to essentially run the program as normal but output as much useful debugging information as we can get away with. While there are a few ways you could do this, the way it works so far is as follows. Anytime a function which is defined in this function, and that we already type information for, we pack the parameters that are being used as inputs into the function into a byte buffer. This buffer is then sent to an external function which handles the logging, along with the name of the function being called. Finally, after this the function is called normally, its output stored, and then sent to a different external function, along with the name of the function that has returned. The end result of this is an output that can be generated which looks a lot like a stacktrace, but of the entire program. Next steps may be something like storing information like types and names about global variables, so that the output debug information can also show cached global variables,<br>

## Commit log
[log.md](./log.md)