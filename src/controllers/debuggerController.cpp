#include "controllerLib.h"

std::ostream* o = &std::cout;

#include <csignal>
#include <csetjmp>
#include <unordered_map>
std::unordered_map<int,const char*> signalCodeToString = {
    {SIGABRT, "SIGABRT"},
    {SIGFPE, "SIGFPE"},
    {SIGILL, "SIGILL"},
    {SIGINT, "SIGINT"},
    {SIGSEGV, "SIGSEGV"},
    {SIGTERM, "SIGTERM"},
    {SIGKILL, "SIGKILL"},
    {SIGQUIT, "SIGQUIT"},
    {SIGTRAP, "SIGTRAP"},
    {SIGBUS, "SIGBUS"},
    {SIGSTOP, "SIGSTOP"},
    {SIGSYS, "SIGSYS"},
    {SIGALRM, "SIGALRM"},
    {SIGCONT, "SIGCONT"}
};
std::string codeToString(int code) {
    return signalCodeToString.count(code)?signalCodeToString[code]:std::to_string(code);
}
sigjmp_buf signalHandledReturn;
int lastSignal = -1;
void signalHandler(int signal) {
    std::cout << "SIGNAL\n";
    lastSignal = signal;
    siglongjmp(signalHandledReturn, 1);
}
void* all_old_handlers[64]={nullptr};
void handleAllSignals() {
    std::cout << "handling signals\n";
    for(int i = 0; i < 64; i++) {
        __sighandler_t tmp = signal(i, signalHandler);
        all_old_handlers[i] = (tmp==SIG_ERR)?nullptr:(void*)tmp;
    }
}
void unhandleSignals() {
    std::cout << "unhandling signals\n";
    for(int i = 0; i < 64; i++) {
        signal(i, (__sighandler_t)all_old_handlers[i]);
        all_old_handlers[i]=nullptr;
    }
}
#include <functional>
bool runFunctionWithSignals(std::function<void()>& possig) {
    handleAllSignals();
    bool sigSent = false;
    if (sigsetjmp(signalHandledReturn, 1) == 0) {
        try {
            possig();
        } catch (...) {
            std::cout << "caught something\n";
        }
    } else {
        sigSent = true;
        *o << "Signal " << codeToString(lastSignal) << " Ocurred.\n";
        lastSignal=-1;
    }
    unhandleSignals();
    return !sigSent;
}

#include <map>
#include <chrono>
std::vector<const char*> funcNameStack;
// tried adding function timing, doesnt work currently for some reason
/*using time_point_steady = std::chrono::time_point<std::chrono::steady_clock>;
using time_duration_steady = std::chrono::duration<double>;
time_point_steady getTime() {
    return std::chrono::steady_clock::now();
}
time_duration_steady getTimeDiff(time_point_steady start) {
    return getTime()-start;
}
std::vector<time_point_steady> funcStartTimes;*/
std::map<std::string, size_t> mangledToIndex;
extern "C" void logFunctionParameters(const char* funcName, void* buffer) {
    if (o == nullptr) return;
    funcNameStack.push_back(funcName);
    //funcStartTimes.push_back(getTime());
    for(int i = 0; i < indentLevel; i++) *o << "    ";
    indentLevel++;
    if (mangledToIndex.count(funcName) > 0) {
        int j = mangledToIndex[funcName];
        unsigned int paramCount = functionParamCounts[j];
        *o << "Function \"" << functionNames[j] << "\" was called";
        if (paramCount > 0) {
            *o << " with parameters {\n";
            unsigned int offset = 0;
            for(int k = 0; k < paramCount; k++) {
                for(int i = 0; i < indentLevel; i++) *o << "    ";
                *o << functionParamNames[j][k] << " = (" << functionParamTypes[j][k] << ")";
                printType(functionParamTypes[j][k], (void*)((char*)buffer+offset), *o);
                offset += getTypeByteLength(functionParamTypes[j][k]);
                *o << "\n";
            }
            if (functionIsVariadic[j]) {
                for(int i = 0; i < indentLevel; i++) *o << "    ";
                int n = *(int*)((char*)buffer+offset);
                offset += sizeof(int);
                if (n > 0) {
                    *o << "... = (" << n << " * T) {\n";
                    indentLevel++;
                    for(int k = 0; k < n; k++) {
                        for(int i = 0; i < indentLevel; i++) *o << "    ";
                        char* type = *(char**)((char*)buffer+offset);
                        *o << '(' << type << ") ";
                        offset += sizeof(void*);
                        printType(type, (void*)((char*)buffer+offset), *o);
                        if (k+1!=n) *o << ',';
                        *o << "\n";
                        offset += getTypeByteLength(type);
                    }
                    indentLevel--;
                    for(int i = 0; i < indentLevel; i++) *o << "    ";
                    *o << "}\n";
                } else {
                    *o << "... = (0 * T) { }\n";
                }
            }
            for(int i = 1; i < indentLevel; i++) *o << "    ";
            *o << '}';
        } else {
            *o << " without parameters";
        }
        *o << '\n';
        return;
    }
    *o << "Function \"" << funcName << "\" was called with unknown parameters\n";
}
extern "C" void logFunctionReturn(const char* funcName, void* buffer) {
    if (o == nullptr) return;
    indentLevel--;
    while (std::strcmp(funcNameStack[funcNameStack.size()-1],funcName)>0) {
        for(int i = 0; i < indentLevel; i++) *o << "    ";
        *o << funcNameStack[funcNameStack.size()-1] << "\" should have returned.\n";
        funcNameStack.pop_back();
        //funcStartTimes.pop_back();
        indentLevel--;
    }
    //time_duration_steady duration = getTimeDiff(funcStartTimes[funcStartTimes.size()-1]);
    funcNameStack.pop_back();
    //funcStartTimes.pop_back();
    for(int i = 0; i < indentLevel; i++) *o << "    ";
    for(int j = 0; j < numFunctions; j++) {
        if (std::strcmp(funcName, functionMangledNames[j]) == 0) {
            *o << "Function \"" << functionNames[j] << "\" returned";// after " << duration.count() << "ms";
            if (std::strcmp(functionReturnTypes[j], "void") != 0) {
                *o << ", output = (" << functionReturnTypes[j] << ")";
                printType(functionReturnTypes[j], buffer, *o);
            }
            *o << "\n";
            return;
        }
    }
    *o << "Function \"" << funcName << "\" returned\n";// after " << duration.count() << "ms\n";
}
int main(int argc, char** argv) {
    for(int i = 0; i < numFunctions; i++)
        mangledToIndex[functionMangledNames[i]] = i;
    // get index of main function and whether it is valid
    int i;
    bool isValid = true;
    for (i = 0; i < numFunctions; i++) {
        if (std::strcmp(functionNames[i], "main") != 0)
            continue;
        for(int j = 0; j < functionParamCounts[i]; j++)
            if (!isInputableType(functionParamTypes[i][j])) {
                isValid = false;
                break;
            }
        break;
    }
    if (!isValid) {
        std::cout << "Invalid main function\n";
        return 1;
    }
    // open file for output
    std::fstream f;
    f.open("./out/output.txt", std::ios::out);
    o = &f;
    // get parameters for main
    bufferWriter parameters;
    std::vector<bufferWriter*> storage;
    if (argc > 1) {
        if (functionParamCounts[i] == 2) {
            parameters.push<int>(argc);
            storage.push_back(new bufferWriter());
            for(unsigned int j = 0; j < argc; j++)
                storage[0]->push<void*>((void*)argv[j]);
            parameters.push<void*>((void*)storage[0]->pointer);
        }
    } else
        for(int j = 0; j < functionParamCounts[i]; j++)
            inputType(functionParamTypes[i][j], parameters, storage, functionParamNames[i][j], false);
    // call main
    logFunctionParameters("main", parameters.pointer);
    bool returnsInt = std::strcmp(functionReturnTypes[i], "int") == 0;
    std::function<void()> func = [returnsInt,i,parameters](){
        if (returnsInt) {
            int output = ((intFT)functionPointers[i])(parameters.pointer);
            logFunctionReturn("main", (void*)&output);
        } else {
            functionPointers[i](parameters.pointer);
            logFunctionReturn("main", nullptr);
        }
    };
    if (!runFunctionWithSignals(func)) {
        for (std::vector<const char*>::const_reverse_iterator i = funcNameStack.crbegin(); i < funcNameStack.crend(); ++i) {
            indentLevel--;
            for(int i = 0; i < indentLevel; i++) *o << "    ";
            *o << "Function \"" << *i << "\" should have returned.\n";
        }
        funcNameStack.clear();
        //funcStartTimes.clear();
    }
    o = nullptr;
    f.close();
    // cleanup
    for(int j = 0; j < storage.size(); j++)
        delete storage[j];
    return 0;
}