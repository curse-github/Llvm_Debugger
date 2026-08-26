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
- The process that is performed by this project consists of 5 main parts. The first two of which are clang plugins, processing the original source file. The next two are LLVM plugins, performing transformations on the LLVM assembly code. Finally there is the “controller” program which reads in the final modified code as a library and can call functions and handle logging intercepts.
- The two parts of the clang plugin are a type definition finder, and a function parameter finder. The type definition finder looks over all the clang AST generated from the source code and looks for type definitions for things like structs, enums, and classes. It then records this information in a file with information like the type definition name, number of contained fields, field names, and then either field names or values. Then, in a separate clang plugin pass, it loops through all function definitions and records the function name, return type, number of parameters, parameter names, and finally parameter types.
- Next, the clang plugins are run, first the librarification pass, and then the logger pass. The librarification pass goes through all function definitions in the LLVM code, and does two main things. First it records all information about the function like name return type and parameter names and types in a global variable lists to be read back in later by the “controller”. Next it creates a wrapper function for the original which takes in a buffer of the parameters of the original function as an argument, decomposes each part, calls the original functions, and then returns the value of the function call. The logger LLVM pass will go through instances of function calls and compose a buffer of data with the parameters being input to the function and give it to an external function “logFunctionParameters” which will be defined in the controller. Then, after the function call, it will pass a pointer to the return value of that function call to another external function “logFunctionReturn”.
- The final part of the process is the controller program. This program has external lists defined for all the needed information about functions, structs, classes, enums, and unions of the librarified original program. Then, depending on the controller used in the compilation process, it may either ask the user what function they would like to call and get inputs for the parameters of the selected function, or it may simply call the main function and utilize the logFunctionParameters and logFunctionReturn to log what functions are being called and with what parameters.


## Commit log
[log.md](./log.md)