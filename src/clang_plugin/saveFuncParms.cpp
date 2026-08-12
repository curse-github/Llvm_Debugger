#include <fstream>

#include "clangHelpers.h"
//#define DEBUG

unsigned int discardedFuncs = 0;
void HandleFunctionDecl(const FunctionDecl* FD, std::fstream* FuncParmsOut) {
    std::string name = FD->getQualifiedNameAsString();
    if (name.find("lambda") != std::string::npos) {
        discardedFuncs++;
#ifdef DEBUG
        std::cout << "disqualifying \"";
        printFunctionSig(FD);
        std::cout << "\" for being a lambda\n";
#endif
        return;
    }
    if (name.find("<") != std::string::npos) {
        discardedFuncs++;
#ifdef DEBUG
        std::cout << "disqualifying \"";
        printFunctionSig(FD);
        std::cout << "\" for being a templated\n";
#endif
        return;
    }
    if (name.find(",") != std::string::npos) {
        discardedFuncs++;
#ifdef DEBUG
        std::cout << "disqualifying \"";
        printFunctionSig(FD);
        std::cout << "\" for containing ','\n";
#endif
        return;
    }
    std::string returnType = typeToString(FD->getReturnType(), true);
    // dont output functions with any unknown types
    if (returnType.starts_with("unknown")) {
        discardedFuncs++;
#ifdef DEBUG
        std::cout << "disqualifying \"";
        printFunctionSig(FD);
        std::cout << "\" for return type \"" << typeToString(FD->getReturnType()) << "\"|\"" << FD->getReturnType().getAsString() << "\"\n";
#endif
        return;
    }
#ifdef DEBUG
    unsigned int num = 0;
#endif
    for (FunctionDecl::param_const_iterator i = FD->param_begin(), e = FD->param_end(); i != e; i++) {
        if (typeToString((**i).getOriginalType()).starts_with("unknown")) {
            discardedFuncs++;
#ifdef DEBUG
            std::cout << "disqualifying \"";
            printFunctionSig(FD);
            std::cout << "\" for parameter#" << num << " type \"" << typeToString((**i).getOriginalType()) << "\"|\"" << (**i).getOriginalType().getAsString() << "\"\n";
#endif
            return;
        }
#ifdef DEBUG
        num++;
#endif
    }
    // output function
    if (const CXXMethodDecl *MD = dyn_cast<CXXMethodDecl>(FD)) {
        std::string className = MD->getThisType().getCanonicalType().getAsString();
        if (className.starts_with("class ")) {
            // remove "class " at the beginning and " *" at the end;
            className = className.substr(6z,className.size()-8z);
            knownClasses[className] = true;
        }
        // output two versions
        // for some struct or class functions you end up with possible types of the function
        // v1: ... class::func(class* this, ...)
        *FuncParmsOut << returnType << ',' << name << ',' << (FD->getNumParams()+1) << ',';
        *FuncParmsOut << typeToString(MD->getThisType()) << ",this,";
        for (FunctionDecl::param_const_iterator i = FD->param_begin(), e = FD->param_end(); i != e; i++) {
            const ParmVarDecl *PD = *i;
            *FuncParmsOut << typeToString(PD->getOriginalType()) << ',' << PD->getName().str() << ',';
        }
        *FuncParmsOut << '\n';
        // and v2: ... class::func(...)
        /**FuncParmsOut << returnType << ',' << name << ',' << FD->getNumParams() << ',';
        for (FunctionDecl::param_const_iterator i = FD->param_begin(), e = FD->param_end(); i != e; i++) {
            const ParmVarDecl *PD = *i;
            *FuncParmsOut << typeToString(PD->getOriginalType()) << ',' << PD->getName().str() << ',';
        }
        *FuncParmsOut << '\n';*/
    } else {
        *FuncParmsOut << returnType << ',' << name << ',' << FD->getNumParams() << ',';
        for (FunctionDecl::param_const_iterator i = FD->param_begin(), e = FD->param_end(); i != e; i++) {
            const ParmVarDecl *PD = *i;
            *FuncParmsOut << typeToString(PD->getOriginalType()) << ',' << PD->getName().str() << ',';
        }
        *FuncParmsOut << '\n';
    }
    return;
}
void readTypeDefs() {
    std::fstream* TypedefsIn = new std::fstream();
    TypedefsIn->open("tmp/typedefs.csv", std::ios::in);
    std::string lineString;
    while(std::getline(*TypedefsIn, lineString, '\n')) {
        std::stringstream lineSStream(lineString);
        std::string type;
        if (!std::getline(lineSStream, type, ',')) break;
        std::string name;
        if (type == "struct") {
            if (!std::getline(lineSStream, name, ',')) break;
            knownStructs[name] = true;
        } else if (type == "enum") {
            if (!std::getline(lineSStream, name, ',')) break;
            if (!std::getline(lineSStream, name, ',')) break;
            knownEnums[name] = true;
        } else
            std::cout << "ERROR!\n";
    }
    TypedefsIn->close();
    delete TypedefsIn;
}

class SaveFuncParmsConsumer : public ASTConsumer {
    std::set<std::string> ParsedTemplates;
    std::fstream* FuncParmsOut;
    std::vector<const FunctionDecl*> foundFunctions;
    public:
    SaveFuncParmsConsumer(std::set<std::string> ParsedTemplates)
        : ParsedTemplates(ParsedTemplates) {
            FuncParmsOut = new std::fstream();
            FuncParmsOut->open("tmp/functionParams.csv", std::ios::app);
            readTypeDefs();
        }
    ~SaveFuncParmsConsumer() {
        for (const FunctionDecl *FD : foundFunctions)
            HandleFunctionDecl(FD, FuncParmsOut);
#ifdef DEBUG
        std::cout << "discarded " << discardedFuncs << " functions\n";
#endif
        FuncParmsOut->close();
        delete FuncParmsOut;
        FuncParmsOut = nullptr;
    }
    std::map<std::string, bool> knownFunctions;
    void handleDecl(Decl* D) {
        if (const FunctionDecl *FD = dyn_cast<FunctionDecl>(D)) {
            std::string name = FD->getQualifiedNameAsString();
            if (knownFunctions.count(name) == 0) {
                knownFunctions[name] = true;
                //std::cout << "found function \"" << name << "\"\n";
                foundFunctions.push_back(FD);
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