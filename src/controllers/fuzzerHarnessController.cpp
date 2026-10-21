#include "controllerLib.h"
#include <map>
#include <set>

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
    for (int i = 0; i <= lastInputtableStruct; i++) {
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
        o << "        }" << (((i+1)<=lastInputtableStruct)?",":"") << '\n';
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
        o << "'a'";
    } else if (type == "char") {
        o << "\"string\"";
    } else if (type == "short") {
        o << "256";
    } else if (type == "int") {
        o << "65536";
    } else if (type == "long") {
        o << "4294967296";
    } else if (type == "float") {
        o << "0.0";
    } else if (type == "double") {
        o << "0.000";
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
std::string getUntilQuote(std::istream& i) {
    std::string out = "";
    char c = ' ';
    i >> c;
    while (c != '"' && c != '\'') {
        out += c;
        i >> c;
    }
    return out;
}
std::string parseNonString(std::istream& i) {
    std::string out;
    char c = ' ';
    char p = (char)i.peek();
    if (p == '\'') {
        i >> c;
        out = p;
        out += getUntilQuote(i);
        out += p;
        return out;
    }
    while (p != ' ' && p != '\n' && p != ',' && p != '}' && p != ']')
        if (i >> c) {
            out += c;
            p = i.peek();
        } else
            std::cout << "error\n";
    return out;
}
bool assertChar(std::istream& i, char t) {
    char c = ' ';
    if (!(i >> c) || c != t) {
        std::cout << "Malformed json.1\n";
        return false;
    }
    return true;
}
std::string harnessParsedFunctionName;
std::map<std::string,std::string> harnessParsedParamList;
bool jsonParseParamList(std::istream& i) {
    if (!assertChar(i,'[')) return false;
    char nextChar = ',';
    while (nextChar == ',') {
        std::string name = "";
        std::string value = "";
        i >> std::ws;
        if (!assertChar(i,'{')) return false;
        i >> std::ws;
        if (!assertChar(i,'"')) return false;
        std::string key1 = getUntilQuote(i);
        i >> std::ws;
        if (!assertChar(i,':')) return false;
        i >> std::ws;
        if (key1 == "Name") {
            if (!assertChar(i,'"')) return false;
            name = getUntilQuote(i);
        } else if (key1 == "Value") {
            value = parseNonString(i);
        } else {
            std::cout << "Malformed json.2\n";
            return false;
        }
        i >> std::ws;
        if (!assertChar(i,',')) return false;
        i >> std::ws;
        if (!assertChar(i,'"')) return false;
        std::string key2 = getUntilQuote(i);
        i >> std::ws;
        if (!assertChar(i,':')) return false;
        i >> std::ws;
        std::string value2 = "";
        if (key2 == "Name") {
            if (!assertChar(i,'"')) return false;
            name = getUntilQuote(i);
        } else if (key2 == "Value") {
            value = parseNonString(i);
        } else {
            std::cout << "Malformed json.3\n";
            return false;
        }
        i >> std::ws;
        if (!assertChar(i,'}')) return false;
        i >> std::ws;
        i >> nextChar;
        if (name.size() == 0 || value.size() == 0) {
            std::cout << "Malformed json.4\n";
            return false;
        }
        harnessParsedParamList[name] = value;
    }
    return nextChar == ']';
}
bool readJsonHarness(std::istream& i) {
    char c = ' ';
    if (!assertChar(i,'{')) return false;
    i >> std::ws;
    if (!assertChar(i,'"')) return false;
    std::string key = getUntilQuote(i);
    i >> std::ws;
    if (!assertChar(i,':')) return false;
    i >> std::ws;
    if (key == "FunctionName") {
        if (!assertChar(i,'"')) return false;
        harnessParsedFunctionName = getUntilQuote(i);
    } else if (key == "ParamList") {
        if (!jsonParseParamList(i)) return false;
    }
    i >> std::ws;
    if (!assertChar(i,',')) return false;
    i >> std::ws;
    if (!assertChar(i,'"')) return false;
    key = getUntilQuote(i);
    i >> std::ws;
    if (!assertChar(i,':')) return false;
    i >> std::ws;
    if (key == "FunctionName") {
        if (!assertChar(i,'"')) return false;
        harnessParsedFunctionName = getUntilQuote(i);
    } else if (key == "ParamList") {
        if (!jsonParseParamList(i)) return false;
    }
    i >> std::ws;
    if (!assertChar(i,'}')) return false;
    return harnessParsedFunctionName.size()>0;
}
bool pushTypeToBuffer(bufferWriter& buff, std::vector<bufferWriter*>& storage, std::string value, std::string type) {
    if (type == "bool") {
        bool finalValue;
        if (value == "true")
            finalValue = true;
        else if (value == "false")
            finalValue = false;
        else {
            std::cout << "failed to parse (" << value << ") as bool\n";
            return false;
        }
        buff.push<bool>(finalValue);
        return true;
    } else if (type == "char") {
        if (value.size() == 3 && value[0] == '\'' && value[2] == '\'') {
            buff.push<char>(value[1]);
            return true;
        } else {
            try {
                unsigned int intValue = std::stoi(value);
                if (intValue < 256) {
                    buff.push<char>((char)intValue);
                    return true;
                } else {
                    std::cout << "failed to parse (" << value << ") as char\n";
                    return false;
                }
            } catch (std::invalid_argument const& e) {
                std::cout << "failed to parse (" << value << ") as char\n";
                return false;
            } catch (std::out_of_range const& e) {
                std::cout << "failed to parse (" << value << ") as char\n";
                return false;
            }
        }
    } else if (type == "char*") {
        if (value[0] == '"' && value[value.size()-1] == '"') {
            int index = storage.size();
            storage.push_back(new bufferWriter{});
            for(int i = 1; i < value.size()-1; i++)
                storage[index]->push<char>(value[i]);
            storage[index]->push<char>('\0');
            buff.push<void*>(storage[index]->pointer);
            return true;
        }
        std::cout << "failed to parse (" << value << ") as char*\n";
        return false;
    } else if (type == "short") {
        try {
            unsigned int intValue = std::stoi(value);
            if (intValue < 65535) {
                buff.push<short>((short)intValue);
                return true;
            } else {
                std::cout << "failed to parse (" << value << ") as short\n";
                return false;
            }
        } catch (std::invalid_argument const& e) {
            std::cout << "failed to parse (" << value << ") as short\n";
            return false;
        } catch (std::out_of_range const& e) {
            std::cout << "failed to parse (" << value << ") as short\n";
            return false;
        }
    } else if (type == "int") {
        try {
            unsigned int intValue = std::stoi(value);
            buff.push<unsigned int>(intValue);
            return true;
        } catch (std::invalid_argument const& e) {
            std::cout << "failed to parse (" << value << ") as int\n";
            return false;
        } catch (std::out_of_range const& e) {
            std::cout << "failed to parse (" << value << ") as int\n";
            return false;
        }
    } else if (type == "long") {
        try {
            unsigned long intValue = std::stol(value);
            buff.push<unsigned long>(intValue);
            return true;
        } catch (std::invalid_argument const& e) {
            std::cout << "failed to parse (" << value << ") as long\n";
            return false;
        } catch (std::out_of_range const& e) {
            std::cout << "failed to parse (" << value << ") as long\n";
            return false;
        }
    } else if (type == "float") {
        try {
            float intValue = std::stof(value);
            buff.push<float>(intValue);
            return true;
        } catch (std::invalid_argument const& e) {
            std::cout << "failed to parse (" << value << ") as float\n";
            return false;
        } catch (std::out_of_range const& e) {
            std::cout << "failed to parse (" << value << ") as float\n";
            return false;
        }
    } else if (type == "double") {
        try {
            double intValue = std::stod(value);
            buff.push<double>(intValue);
            return true;
        } catch (std::invalid_argument const& e) {
            std::cout << "failed to parse (" << value << ") as float\n";
            return false;
        } catch (std::out_of_range const& e) {
            std::cout << "failed to parse (" << value << ") as float\n";
            return false;
        }
    } else {
        return false;
    }
}
void runHarness() {
    int i;
    for (i = 0; i < numFunctions; i++)
        if (functionNames[i] == harnessParsedFunctionName)
            break;
    if (i == numFunctions) {
        std::cout << "could not find function \"" << harnessParsedFunctionName << "\"\n";
        return;
    }
    for (int j = 0; j < functionParamCounts[i]; j++) {
        if (harnessParsedParamList.find(functionParamNames[i][j]) == harnessParsedParamList.end()) {
            std::cout << "Missing function argument \"" << functionParamNames[i][j] << "\"\n";
            return;
        }
    }
    std::cout << "running function \"" << harnessParsedFunctionName << '(';
    for (int j = 0; j < functionParamCounts[i]; j++)
        std::cout << harnessParsedParamList[functionParamNames[i][j]] << (((j+1)!=functionParamCounts[i])?", ":"");
    std::cout << ")\"\n";
    bufferWriter parameters;
    std::vector<bufferWriter*> storage;
    for (int j = 0; j < functionParamCounts[i]; j++)
        if (!pushTypeToBuffer(parameters, storage, harnessParsedParamList[functionParamNames[i][j]], functionParamTypes[i][j]))
            return;
    std::string functionReturnType = functionReturnTypes[i];
    if (std::strcmp(functionReturnType.c_str(), "bool") == 0) {
        bool output = ((boolFT)functionPointers[i])(parameters.pointer);
        std::cout << "{\n    \"Return\":{\n        \"Type\":\"bool\",\n        \"Value\":" << (output?"true":"false") << "\n    }\n}\n";
    } else if (std::strcmp(functionReturnType.c_str(), "char") == 0) {
        char output = ((charFT)functionPointers[i])(parameters.pointer);
        std::cout << "{\n    \"Return\":{\n        \"Type\":\"char\",\n        \"Value\":" << output << "\n    }\n}\n";
    } else if (std::strcmp(functionReturnType.c_str(), "char*") == 0) {
        const char* output = ((stringFT)functionPointers[i])(parameters.pointer);
        std::cout << "{\n    \"Return\":{\n        \"Type\":\"char*\",\n        \"Value\":\"" << output << "\"\n    }\n}\n";
    } else if (std::strcmp(functionReturnType.c_str(), "short") == 0) {
        short output = ((shortFT)functionPointers[i])(parameters.pointer);
        std::cout << "{\n    \"Return\":{\n        \"Type\":\"short\",\n        \"Value\":" << output << "\n    }\n}\n";
    } else if (std::strcmp(functionReturnType.c_str(), "int") == 0) {
        int output = ((intFT)functionPointers[i])(parameters.pointer);
        std::cout << "{\n    \"Return\":{\n        \"Type\":\"int\",\n        \"Value\":" << output << "\n    }\n}\n";
    } else if (std::strcmp(functionReturnType.c_str(), "long") == 0) {
        long output = ((longFT)functionPointers[i])(parameters.pointer);
        std::cout << "{\n    \"Return\":{\n        \"Type\":\"long\",\n        \"Value\":" << output << "\n    }\n}\n";
    } else if (std::strcmp(functionReturnType.c_str(), "float") == 0) {
        float output = ((floatFT)functionPointers[i])(parameters.pointer);
        std::cout << "{\n    \"Return\":{\n        \"Type\":\"float\",\n        \"Value\":" << output << "\n    }\n}\n";
    } else if (std::strcmp(functionReturnType.c_str(), "double") == 0) {
        double output = ((doubleFT)functionPointers[i])(parameters.pointer);
        std::cout << "{\n    \"Return\":{\n        \"Type\":\"double\",\n        \"Value\":" << output << "\n    }\n}\n";
    } else {
        functionPointers[i](parameters.pointer);
        std::cout << "{\n    \"Return\":{\n        \"Type\":\"void\"\n    }\n}\n";
    }
}
bool setAutoHarness(std::string functionName) {
    harnessParsedFunctionName = functionName;
    int i;
    for (i = 0; i < numFunctions; i++)
        if (functionNames[i] == functionName)
            break;
    if (i == numFunctions) {
        std::cout << "Unable to find function \"" << functionName << "\"\n";
        return false;
    }
    for (int j = 0; j < functionParamCounts[i]; j++) {
        std::string type = functionParamTypes[i][j];
        if (type == "bool") {
            harnessParsedParamList[functionParamNames[i][j]] = "false";
        } else if (type == "char") {
            harnessParsedParamList[functionParamNames[i][j]] = "'a'";
        } else if (type == "char*") {
            harnessParsedParamList[functionParamNames[i][j]] = "\"string\"";
        } else if (type == "short") {
            harnessParsedParamList[functionParamNames[i][j]] = "256";
        } else if (type == "int") {
            harnessParsedParamList[functionParamNames[i][j]] = "65536";
        } else if (type == "long") {
            harnessParsedParamList[functionParamNames[i][j]] = "4294967296";
        } else if (type == "float") {
            harnessParsedParamList[functionParamNames[i][j]] = "0.0";
        } else if (type == "double") {
            harnessParsedParamList[functionParamNames[i][j]] = "0.000";
        } else {
            std::cout << "Unable to make harness for function \"" << functionName << "\"\n";
            return false;
        }
    }
    return true;
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
    if (argc <= 1) {
        std::cout << "Enter \"<program> help\" for options.\n";
        return 1;
    }
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
                std::cout << "Unknown option \"" << argv[2] << "\"\n";
                std::cout << "Enter \"<program> help\" for options.\n";
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
            if (argc >= 3)
                std::cout << "Unknown option \"" << argv[2] << "\"\n";
            std::cout << "Enter \"<program> help\" for options.\n";
            return 1;
        }
    } else if (std::strcmp(argv[1],"harness")==0) {
        if ((argc != 3) && (argc != 5)) {
            std::cout << "Malformed arguments.\n";
            if (argc >= 3)
                std::cout << "Unknown option \"" << argv[2] << "\"\n";
            std::cout << "Enter \"<program> help\" for options.\n";
            return 1;
        }
        if (argc == 3) {
            printHarness(std::cout,argv[2]);
            return 0;
        } else {
            if (std::strcmp(argv[2],"-o") == 0 || std::strcmp(argv[2],"-O") == 0) {
                std::fstream f;
                f.open(argv[3], std::ios::out);
                printHarness(f,argv[4]);
                f.close();
                return 0;
            } else if (std::strcmp(argv[3],"-o") == 0 || std::strcmp(argv[3],"-O") == 0) {
                std::fstream f;
                f.open(argv[4], std::ios::out);
                printHarness(f,argv[2]);
                f.close();
                return 0;
            } else {
                std::cout << "Malformed arguments.\n";
                if (argc >= 3)
                    std::cout << "Unknown option \"" << argv[2] << "\"\n";
                std::cout << "Enter \"<program> help\" for options.\n";
                return 1;
            }
        }
    } else if (std::strcmp(argv[1],"run_harness")==0) {
        if ((argc != 2) && (argc != 4)) {
            std::cout << "Malformed arguments.\n";
            if (argc >= 3)
                std::cout << "Unknown option \"" << argv[2] << "\"\n";
            std::cout << "Enter \"<program> help\" for options.\n";
            return 1;
        }
        if (argc == 2) {
            if (!readJsonHarness(std::cin)) {
                std::cout << "Failed to parse JSON.\n";
                return 1;
            }
            runHarness();
            return 0;
        } else {//  argc == 4
            if (std::strcmp(argv[2],"-i") == 0 || std::strcmp(argv[2],"-I") == 0) {
                std::fstream f;
                std::cout << "input file: \"" << argv[3] << "\"\n";
                f.open(argv[3], std::ios::in);
                if (!readJsonHarness(f)) {
                    f.close();
                    std::cout << "Failed to parse JSON.\n";
                    return 1;
                }
                f.close();
                runHarness();
                return 0;
            } else {
                std::cout << "Malformed arguments.\n";
                std::cout << "Unknown option \"" << argv[2] << "\"\n";
                std::cout << "Enter \"<program> help\" for options.\n";
                return 1;
            }
        }
    } else if (std::strcmp(argv[1],"run_auto")==0) {
        if (argc < 3) {
            std::cout << "Malformed arguments.\n";
            return 1;
        }
        if (setAutoHarness(argv[2]))
            runHarness();
        return 0;
    } else if (std::strcmp(argv[1],"run_auto_all")==0) {
        std::set<std::string> completedFunctions;
        for (int i = 0; i < numFunctions; i++) {
            if (completedFunctions.count(functionNames[i]) != 0)
                continue;
            completedFunctions.insert(functionNames[i]);
            if (setAutoHarness(functionNames[i]))
                runHarness();
        }
        return 0;
    }
    std::cout << "Unknown option \"" << argv[1] << "\"\n";
    std::cout << "Enter \"<program> help\" for options.\n";
    return 1;
}