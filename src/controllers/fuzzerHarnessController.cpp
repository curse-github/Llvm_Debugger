#include "controllerLib.h"
#include <map>

std::vector<bool> isFunctionValid;
std::vector<bool> canPrintFunctionOutput;
void printSpec(std::ostream& o) {
    typedef std::map<std::string,int> uniqueNamesType;
    uniqueNamesType uniqueNames;
    for (int i = 0; i < numFunctions; i++) {
        if (!isFunctionValid[i]) continue;
        if (uniqueNames.find(functionNames[i]) == uniqueNames.end())
            uniqueNames[functionNames[i]] = 1;
        else
            uniqueNames[functionNames[i]]++;
    }
    o << "{\n";
    o << "    \"Functions\":{\n";
    for (uniqueNamesType::const_iterator I = uniqueNames.cbegin(), E = uniqueNames.cend(); I != E; ++I) {
        std::string functionName = I->first;
        int numOverloads = I->second;
        int numFound = 0;
        o << "        \"" << functionName << "\":{\n";
        o << "            \"Overloads\":[\n";
        for (int i = 0; i < numFunctions; i++) {
            if (!isFunctionValid[i]) continue;
            if (functionNames[i] != functionName) continue;
            numFound++;
            o << "                {\n";
            if (functionParamCounts[i] > 0) {
                o << "                    \"ParamList\":[\n";
                for (int j = 0; j < functionParamCounts[i]; j++) {
                    o << "                        {\n";
                    o << "                            \"Name\":\"" << functionParamNames[i][j] << "\",\n";
                    o << "                            \"Type\":\"" << functionParamTypes[i][j] << "\"\n";
                    o << "                        }" << (((j+1)!=functionParamCounts[i])?",":"") << '\n';
                }
                o << "                    ],\n";
            } else {
                o << "                    \"ParamList\":[],\n";
            }
            o << "                    \"Return\":{\n";
            o << "                        \"Type\":\"" << functionReturnTypes[i] << "\"\n";
            o << "                    }\n";
            o << "                }" << ((numFound != numOverloads)?",":"") << '\n';
        }
        o << "            ]\n";
        o << "        }" << ((std::next(I)!=E)?",":"") << '\n';
    }
    o << "    },\n";
    int lastInputtableStruct = -1;
    for (int i = 0; i < numStructs; i++)
        if (isInputableType(structNames[i],false))
            lastInputtableStruct = i;
    o << "    \"Structs\":{\n";
    for (int i = 0; i < lastInputtableStruct; i++) {
        if (!isInputableType(structNames[i],false)) continue;
        o << "        \"" << structNames[i] << "\":{\n";
        o << "            \"Fields\":[\n";
        for (int j = 0; j < structNumFields[i]; j++) {
            o << "                {\n";
            o << "                    \"Name\":\"" << structFieldNames[i][j] << "\",\n";
            o << "                    \"Type\":\"" << structFieldTypes[i][j] << "\"\n";
            o << "                }" << (((j+1)!=structNumFields[i])?",":"") << '\n';
        }
        o << "            ]\n";
        o << "        }" << (((i+1)!=lastInputtableStruct)?",":"") << '\n';
    }
    o << "    },\n";
    o << "    \"Enums\":{\n";
    for (int i = 0; i < numEnums; i++) {
        o << "        \"" << enumNames[i] << "\":{\n";
        o << "            \"Options\":[\n";
        for (int j = 0; j < enumNumValues[i]; j++)
            o << "                \"" << enumValueNames[i][j] << "\"" << (((j+1)!=enumNumValues[i])?",":"") << '\n';
        o << "            ]\n";
        o << "        }" << (((i+1)!=numEnums)?",":"") << '\n';
    }
    o << "    }\n";
    o << "}\n";
}
void printDefaultValues(std::ostream& o, std::string type) {
    if (type == "bool") {
        o << "false";
    } else if (type == "char") {
        o << "' '";// because \0 might break some things
    } else if (type == "short") {
        o << 0;
    } else if (type == "int") {
        o << 0;
    } else if (type == "long") {
        o << 0;
    } else if (type == "float") {
        o << "0.0";
    } else if (type == "double") {
        o << "0.0";
    }
}
void printHarness(std::ostream& o, std::string functionName) {
    int i;
    for (i = 0; i < numFunctions; i++)
        if (functionNames[i] == functionName)
            break;
    if (functionNames[i] != functionName) {
        std::cout << "Could not find function \"" << functionName << "\"\n";
        return;
    }
    o << "{\n";
    o << "    \"FunctionName\":\"" << functionName << "\",\n";
    o << "    \"ParamList\":[\n";
    for (int j = 0; j < functionParamCounts[i]; j++) {
        o << "        {\n";
        o << "            \"Name\":\"" << functionParamNames[i][j] << "\",\n";
        o << "            \"Value\":";
        printDefaultValues(o, functionParamTypes[i][j]);
        o << "\n";
        o << "        }" << (((j+1)!=functionParamCounts[i])?",":"") << '\n';
    }
    o << "    ]\n";
    o << "}\n";
}
int main(int argc, char** argv) {
    size_t numValid = 0ull;
    for (unsigned int i = 0; i < numFunctions; i++) {
        const std::string functionReturnType = functionReturnTypes[i];
        bool isValid = true;
        unsigned int paramCount = functionParamCounts[i];
        for(int j = 0; isValid && (j < paramCount); j++)
            isValid = isValid && isInputableType(functionParamTypes[i][j],false);
        isFunctionValid.push_back(isValid);
        canPrintFunctionOutput.push_back((functionReturnType != "unknown") && (functionReturnType[functionReturnType.size()-1] != '*'));
        if (isValid) numValid++;
    }
    if (argc > 1) {
        if (std::strcmp(argv[1],"help")==0) {
            std::cout << "options:\n";
            std::cout << "     <program> help\n";
            std::cout << "         will list program options\n";
            std::cout << "     <program> run <arg1,arg2,arg3,...>\n";
            std::cout << "         will run the compiled program main function with the arguments supplied, minus the \"run\" option\n";
            std::cout << "     <program> manifest\n";
            std::cout << "         will show full program breakdown with types and all functions\n";
            std::cout << "     <program> harness <function>\n";
            std::cout << "         will show example harness json data that can be inputted into run_harness\n";
            std::cout << "     <program> run_harness <function>\n";
            std::cout << "         will run <function> within the compiled program with arguments obtained through cin\n";
            std::cout << "     <program> run_auto <function>\n";
            std::cout << "         will run <function> within the compiled program with default arguments similar to <program> harness <function>\n";
            std::cout << "     <program> run_auto_all\n";
            std::cout << "         will run all functions within the program with default arguments.\n";
            std::cout << "any option without console output can be pass -o <output file> and will instead output to the file\n";
            std::cout << "any option without console input can be pass -i <input file> and will use input from the file\n";
            return 0;
        } else if (std::strcmp(argv[1],"run")==0) {
            int i;
            for (i = 0; i < numFunctions; i++)
                if (std::strcmp(functionNames[i], "main") == 0)
                    break;
            if (std::strcmp(functionNames[i],"main") != 0) {
                std::cout << "Malformed main function.\n";
                return 1;
            }
            bool returnsInt = std::strcmp(functionReturnTypes[i], "int") == 0;
            if (functionParamCounts[i] == 2) {
                int newArgc = argc-1;
                std::vector<const char*> newArgv;
                if (argc > 0) {
                    newArgv.push_back(argv[0]);
                    for (int j = 2; j < argc; j++)
                        newArgv.push_back(argv[j]);
                }
                bufferWriter parameters;
                parameters.push<int>(newArgc);
                parameters.push<void*>((newArgv.size()!=0)?newArgv.data():nullptr);
                if (returnsInt)
                    return ((intFT)functionPointers[i])(parameters.pointer);
                else {
                    functionPointers[i](parameters.pointer);
                    return 0;
                }
            } else if (functionParamCounts[i] == 0) {
                if (returnsInt)
                    return ((intFT)functionPointers[i])(nullptr);
                else {
                    functionPointers[i](nullptr);
                    return 0;
                }
            } else {
                std::cout << "Malformed main function.\n";
            }
        } else if (std::strcmp(argv[1],"manifest")==0) {
            if (argc == 4) {
                if (!(std::strcmp(argv[2],"-o") == 0 || std::strcmp(argv[2],"-O") == 0)) {
                    std::cout << "Malformed arguments.\n";
                    return 1;
                }
                std::fstream f;
                f.open(argv[3], std::ios::out);
                printSpec(f);
                f.close();
                return 0;
            } else if (argc == 2) {
                printSpec(std::cout);
                return 0;
            } else {
                std::cout << "Malformed arguments.\n";
                return 1;
            }
        } else if (std::strcmp(argv[1],"harness")==0) {
            if ((argc != 3) && (argc != 5)) {
                std::cout << "Malformed arguments.\n";
                return 1;
            }
            if (argc == 3) {
                printHarness(std::cout,argv[2]);
            } else {
                if (std::strcmp(argv[2],"-o") == 0 || std::strcmp(argv[2],"-O") == 0) {
                    std::fstream f;
                    f.open(argv[3], std::ios::out);
                    printHarness(f,argv[4]);
                    f.close();
                } else if (std::strcmp(argv[3],"-o") == 0 || std::strcmp(argv[3],"-O") == 0) {
                    std::fstream f;
                    f.open(argv[4], std::ios::out);
                    printHarness(f,argv[2]);
                    f.close();
                } else {
                    std::cout << "Malformed arguments.\n";
                    return 1;
                }
            }
            return 1;
        } else if (std::strcmp(argv[1],"run_harness")==0) {
            std::cout << "Not yet supported.\n";
        } else if (std::strcmp(argv[1],"run_auto")==0) {
            std::cout << "Not yet supported.\n";
        } else if (std::strcmp(argv[1],"run_auto_all")==0) {
            std::cout << "Not yet supported.\n";
        }
        std::cout << "Unknown option \"" << argv[1] << "\"\n";
        std::cout << "Enter \"<program> help\" for options.\n";
        return 1;
    }
    std::cout << "Enter \"<program> help\" for options.\n";
    return 1;
}