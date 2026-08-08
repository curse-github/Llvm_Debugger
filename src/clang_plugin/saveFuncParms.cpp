#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Sema/Sema.h"
using namespace clang;

#include <iostream>
#include <fstream>

class SaveFuncParmsConsumer : public ASTConsumer {
    std::set<std::string> ParsedTemplates;
    std::fstream* outFile;
    public:
    SaveFuncParmsConsumer(std::set<std::string> ParsedTemplates)
        : ParsedTemplates(ParsedTemplates) {
            outFile = new std::fstream();
            outFile->open("tmp/functionParams.csv", std::ios::app);
        }
    ~SaveFuncParmsConsumer() {
        outFile->close();
        delete outFile;
        outFile = nullptr;
    }
    std::map<std::string, bool> knownStructs;
    std::map<std::string, bool> knownClasses;
    std::string fixTypeName(std::string inputT, bool allowVoid=false) {
        if (inputT.find(",") != std::string::npos)
            return "unknown";
        if (inputT.starts_with("const "))
            return fixTypeName(inputT.substr(6ull,inputT.size()-6ull));
        if (inputT.starts_with("volatile "))
            return fixTypeName(inputT.substr(9ull,inputT.size()-9ull));
        if (inputT.starts_with("class "))
            return fixTypeName(inputT.substr(6ull,inputT.size()-6ull));
        if (inputT.starts_with("unsigned "))
            return fixTypeName(inputT.substr(9ull,inputT.size()-9ull));
        char secondLastChar = inputT[inputT.size()-2ull];
        char lastChar = inputT[inputT.size()-1ull];
        if ((lastChar == '&') || (lastChar == '*')) {
            std::string tmp;
            if ((lastChar == '&') && (secondLastChar == '&'))
                tmp = fixTypeName(inputT.substr(0,inputT.size()-2), lastChar == '*');// allow void* but not void&
            else
                tmp = fixTypeName(inputT.substr(0,inputT.size()-1), lastChar == '*');// allow void* but not void&
            if (tmp == "unknown")
                return "unknown";
            return tmp+'*';
        }
        if (lastChar == ']') {// is an array type
            size_t str_i = inputT.find_last_of('[');
            std::string newType = inputT.substr(0, str_i);
            if (str_i+2 == inputT.size()) // ends with []
                return "unknown";
            int count = std::stoi(inputT.substr(str_i+1, inputT.size()-str_i-2));
            std::string tmp = fixTypeName(newType);
            if (tmp == "unknown")
                return "unknown";
            return tmp + '['+std::to_string(count)+"]*";
        }
        std::string outputT = "";
        for (const char c : inputT) {
            if (c == '<')// likely a templated type, and im not sure how to deal with those yet
                return "unknown";
            if (c != ' ')
                outputT += c;
        }
        if (knownStructs.count(outputT) > 0) return "struct."+outputT;
        if (knownClasses.count(outputT) > 0) return "class."+outputT;
        if (outputT == "_Bool") return "bool";
        if (outputT == "longlong") return "long";
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
        return "unknown";
    }
    std::string typeToString(const QualType& T, bool allowVoid=false) {
        return fixTypeName(T.getCanonicalType().getAsString(), allowVoid);
    }
    void HandleFunctionDecl(const FunctionDecl* FD) {
        /*std::cout << "\"" << FD->getReturnType().getAsString() << ' ' << FD->getNameAsString() << '(';
        for (FunctionDecl::param_const_iterator i = FD->param_begin(), e = FD->param_end(); i != e; i++) {
            const ParmVarDecl *PD = *i;
            std::cout << PD->getOriginalType().getAsString() << ' ' << PD->getName().str();
            if (i+1!=e) std::cout << ", ";
        }
        std::cout << ")\"\n";*/
        std::string name = FD->getQualifiedNameAsString();
        if (name.find("lambda") != std::string::npos) {
            //std::cout << "disqualifying \"" << name << "\" for being a lambda\n";
            return;
        }
        if (name.find("<") != std::string::npos) {
            //std::cout << "disqualifying \"" << name << "\" for being a templated\n";
            return;
        }
        if (name.find(",") != std::string::npos) {
            //std::cout << "disqualifying \"" << name << "\" for containing ','\n";
            return;
        }
        std::string returnType = typeToString(FD->getReturnType(), true);
        // dont output functions with any unknown types
        if (returnType == "unknown") {
            //std::cout << "disqualifying \"" << name << "\" for return type \"" << FD->getReturnType().getCanonicalType().getAsString() << "\"\n";
            return;
        }
        //unsigned int num = 0;
        for (FunctionDecl::param_const_iterator i = FD->param_begin(), e = FD->param_end(); i != e; i++) {
            if (typeToString((**i).getOriginalType()) == "unknown") {
                //std::cout << "disqualifying \"" << name << "\" for parameter#" << num << " type \"" << (**i).getOriginalType().getCanonicalType().getAsString() << "\"\n";
                return;
            }
            //num++;
        }
        // output function
        if (const CXXMethodDecl *MD = dyn_cast<CXXMethodDecl>(FD)) {
            std::string className = MD->getThisType().getCanonicalType().getAsString();
            if (className.starts_with("class ")) {
                // remove "class " at the beginning and " *" at the end;
                className = className.substr(6ull,className.size()-8ull);
                knownClasses[className] = true;
            }
            // output two versions
            // v1
            *outFile << returnType << ',' << name << ',' << FD->getNumParams() << ',';
            for (FunctionDecl::param_const_iterator i = FD->param_begin(), e = FD->param_end(); i != e; i++) {
                const ParmVarDecl *PD = *i;
                *outFile << typeToString(PD->getOriginalType()) << ',' << PD->getName().str() << ',';
            }
            *outFile << '\n';
            // v2
            *outFile << returnType << ',' << name << ',' << (FD->getNumParams()+1) << ',';
            *outFile << typeToString(MD->getThisType()) << ",this,";
            for (FunctionDecl::param_const_iterator i = FD->param_begin(), e = FD->param_end(); i != e; i++) {
                const ParmVarDecl *PD = *i;
                *outFile << typeToString(PD->getOriginalType()) << ',' << PD->getName().str() << ',';
            }
            *outFile << '\n';
        } else {
            *outFile << returnType << ',' << name << ',' << FD->getNumParams() << ',';
            for (FunctionDecl::param_const_iterator i = FD->param_begin(), e = FD->param_end(); i != e; i++) {
                const ParmVarDecl *PD = *i;
                *outFile << typeToString(PD->getOriginalType()) << ',' << PD->getName().str() << ',';
            }
            *outFile << '\n';
        }
        return;
    }
    std::map<std::string, bool> knownFunctions;
    void handleDecl(Decl* D) {
        if (const FunctionDecl *FD = dyn_cast<FunctionDecl>(D)) {
            std::string name = FD->getQualifiedNameAsString();
            if (knownFunctions.count(name) == 0) {
                knownFunctions[name] = true;
                //std::cout << "found function \"" << name << "\"\n";
                HandleFunctionDecl(FD);
            }
        } else if (const TypedefDecl *TdD = dyn_cast<TypedefDecl>(D)) {
            std::string typeStr = TdD->getUnderlyingType().getAsString();
            if (typeStr.starts_with("struct ") && (typeStr.find("<") == std::string::npos)) {
                //std::cout << "found struct\n";
                knownStructs[typeStr.substr(7ull, typeStr.size()-7ull)] = true;
            }
        }
    }

    bool HandleTopLevelDecl(DeclGroupRef DG) override {
        for (DeclGroupRef::iterator i = DG.begin(), e = DG.end(); i != e; ++i)
            handleDecl(*i);
        return true;
    }
    void HandleTranslationUnit(ASTContext &Ctx) override {
        struct NamespaceVisitor : public RecursiveASTVisitor<NamespaceVisitor> {
            NamespaceVisitor() { }
            bool VisitDecl(Decl *D) {
                Decls.insert(D);
                return true;
            }
            std::set<Decl*> Decls;
        } v;
        v.TraverseDecl(Ctx.getTranslationUnitDecl());
        //std::cout << "test \"" << Ctx.getTranslationUnitDecl().getQualifiedNameAsString() << "\"\n";
        for (Decl *D : v.Decls)
            handleDecl(D);
    }
};
class SaveFuncParmsAction : public PluginASTAction {
    std::set<std::string> ParsedTemplates;
    protected:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, llvm::StringRef) override {
        return std::make_unique<SaveFuncParmsConsumer>(ParsedTemplates);
    }
    bool ParseArgs(const CompilerInstance &CI, const std::vector<std::string> &args) override {
        return true;
    }
};

static FrontendPluginRegistry::Add<SaveFuncParmsAction> X("save-func-parms", "print function names");