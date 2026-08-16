#include "clangHelpers.h"

std::map<std::string, bool> knownEnums;
std::map<std::string, bool> knownStructs;
std::map<std::string, bool> knownClasses;
std::map<std::string, bool> knownUnions;

std::string fixTypeName(std::string inputT, bool allowVoid) {
    for (const char c : inputT) {
        if (c == '<')// likely a templated type, and im not sure how to deal with those yet
            return "unknown";
        else if ((c == '(') || (c == '}'))
            return "unknown";
    }
    if (inputT.find(",") != std::string::npos)
        return "unknown";
    if (inputT.starts_with("const "))
        return fixTypeName(inputT.substr(6z,inputT.size()-6z));
    if (inputT.starts_with("volatile "))
        return fixTypeName(inputT.substr(9z,inputT.size()-9z));
    if (inputT.starts_with("class "))
        return fixTypeName(inputT.substr(6z,inputT.size()-6z));
    if (inputT.starts_with("unsigned "))
        return fixTypeName(inputT.substr(9z,inputT.size()-9z));
    if (inputT.starts_with("struct "))
        return fixTypeName(inputT.substr(7z,inputT.size()-7z));
    if (inputT.starts_with("enum "))
        return fixTypeName(inputT.substr(5z,inputT.size()-5z));
    if (inputT.ends_with("__restrict"))
        return fixTypeName(inputT.substr(0z,inputT.size()-10z));
    if (inputT.ends_with("&&")) {
        std::string tmp = fixTypeName(inputT.substr(0z,inputT.size()-2z));
        if (tmp.starts_with("unknown"))
            return tmp;
        return tmp+'*';
    }
    if (inputT.ends_with("const *")) {
        std::string tmp = fixTypeName(inputT.substr(0z,inputT.size()-7z), true);
        if (tmp.starts_with("unknown"))
            return tmp;
        return tmp+'*';
    }
    if (inputT.ends_with("*") || inputT.ends_with("&")) {
        std::string tmp = fixTypeName(inputT.substr(0z,inputT.size()-1z), true);
        if (tmp.starts_with("unknown"))
            return tmp;
        return tmp+'*';
    }
    if (inputT.ends_with("]")) {// is an array type
        size_t str_i = inputT.find_last_of('[');
        std::string newType = inputT.substr(0, str_i);
        if ((inputT.size()-str_i-2) == 0) // ends with []
            return "unknown[]";
        int count = std::stoi(inputT.substr(str_i+1, inputT.size()-str_i-2));
        std::string tmp = fixTypeName(newType);
        if (tmp.starts_with("unknown"))
            return tmp;
        return tmp + '['+std::to_string(count)+"]";
    }
    std::string outputT = "";
    for (const char c : inputT) {
        if (c != ' ')
            outputT += c;
    }
    if (knownStructs.count(outputT) > 0) return outputT;
    if (knownClasses.count(outputT) > 0) return outputT;
    if (knownEnums.count(outputT) > 0) return outputT;
    if (knownUnions.count(outputT) > 0) return outputT;
    if (outputT == "_Bool") return "bool";
    if (outputT == "longlong") return "long";
    if (outputT == "longdouble") return "double";
    // if (outputT == "longdouble") return "long double";// 10 byte floating point
    if (
        (outputT == "bool") ||
        (outputT == "char") ||
        (outputT == "short") ||
        (outputT == "int") ||
        (outputT == "long") ||
        (outputT == "float") ||
        (outputT == "double")
    ) return outputT;
    if (allowVoid && (outputT == "void"))
        return outputT;
    // if (outputT != "unknown") std::cout << "discarded type \"" << outputT << "\"\n";
    return "unknown." + outputT;
}
std::string typeToString(const QualType& T, bool allowVoid) {
    return fixTypeName(T.getCanonicalType().getAsString(), allowVoid);
}
void printFunctionSig(const FunctionDecl* FD) {
    std::cout << FD->getReturnType().getCanonicalType().getAsString() << ' ' << FD->getQualifiedNameAsString() << '(';
    for (FunctionDecl::param_const_iterator i = FD->param_begin(), e = FD->param_end(); i != e; i++) {
        const ParmVarDecl *PD = *i;
        std::cout << '"' << typeToString(PD->getOriginalType()) << "\"|\"" << PD->getOriginalType().getCanonicalType().getAsString() << "\" \"" << PD->getName().str() << '"';
        if (i+1 != e) std::cout << ", ";
    }
    std::cout << ")";
}