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
    CompilerInstance &Instance;
    std::set<std::string> ParsedTemplates;
    std::fstream* outFile;
    public:
    SaveFuncParmsConsumer(CompilerInstance &Instance, std::set<std::string> ParsedTemplates)
        : Instance(Instance), ParsedTemplates(ParsedTemplates) {
            outFile = new std::fstream();
            outFile->open("tmp/functionParams.csv", std::ios::app);
        }
    ~SaveFuncParmsConsumer() {
        outFile->close();
        delete outFile;
        outFile = nullptr;
    }

    std::string fixTypeName(std::string inputT) {
        if (inputT.starts_with("const "))
            return fixTypeName(inputT.substr(6ull,inputT.size()-6ull));
        if (inputT.starts_with("volatile "))
            return fixTypeName(inputT.substr(9ull,inputT.size()-9ull));
        if (inputT.starts_with("class "))
            return fixTypeName(inputT.substr(6ull,inputT.size()-6ull));
        if (inputT.starts_with("unsigned "))
            return fixTypeName(inputT.substr(9ull,inputT.size()-9ull));
        char lastChar = inputT[inputT.size()-1];
        if ((lastChar == '&') || (lastChar == '*')) {
            std::string tmp = fixTypeName(inputT.substr(0,inputT.size()-1));
            if (tmp == "unknown")
                return "unknown";
            return tmp+'*';
        }
        std::string outputT = "";
        for (const char c : inputT) {
            if (c == '<')// likely a templated type, and im not sure how to deal with those yet
                return "unknown";
            if (c != ' ')
                outputT += c;
        }
        if (outputT == "_Bool")
            return "bool";
        return outputT;
    }
    std::string typeToString(const QualType& T) {
        return fixTypeName(T.getCanonicalType().getAsString());
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
        if (name.find("lambda") != std::string::npos)
            return;
        *outFile << typeToString(FD->getReturnType()) << ',' << name << ',' << FD->getNumParams() << ',';
        for (FunctionDecl::param_const_iterator i = FD->param_begin(), e = FD->param_end(); i != e; i++) {
            const ParmVarDecl *PD = *i;
            *outFile << typeToString(PD->getOriginalType()) << ',' << PD->getName().str() << ',';
        }
        *outFile << '\n';
    }

    bool HandleTopLevelDecl(DeclGroupRef DG) override {
        for (DeclGroupRef::iterator i = DG.begin(), e = DG.end(); i != e; ++i) {
            const Decl *D = *i;
            if (const FunctionDecl *FD = dyn_cast<FunctionDecl>(D))
                HandleFunctionDecl(FD);
        }
        return true;
    }
    void HandleTranslationUnit(ASTContext& context) override {
        if (!Instance.getLangOpts().DelayedTemplateParsing)
            return;
        struct Visitor : public RecursiveASTVisitor<Visitor> {
            const std::set<std::string> &ParsedTemplates;
            Visitor(const std::set<std::string> &ParsedTemplates) : ParsedTemplates(ParsedTemplates) { }
            bool VisitFunctionDecl(FunctionDecl *FD) {
                if (FD->isLateTemplateParsed() &&
                    ParsedTemplates.count(FD->getNameAsString()))
                LateParsedDecls.insert(FD);
                return true;
            }
            std::set<FunctionDecl*> LateParsedDecls;
        } v(ParsedTemplates);
        v.TraverseDecl(context.getTranslationUnitDecl());
        clang::Sema &sema = Instance.getSema();
        for (const FunctionDecl *FD : v.LateParsedDecls) {
            clang::LateParsedTemplate &LPT = *sema.LateParsedTemplateMap.find(FD)->second;
            sema.LateTemplateParser(sema.OpaqueParser, LPT);
            HandleFunctionDecl(FD);
        }
    }
};
class SaveFuncParmsAction : public PluginASTAction {
    std::set<std::string> ParsedTemplates;
    protected:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, llvm::StringRef) override {
        return std::make_unique<SaveFuncParmsConsumer>(CI, ParsedTemplates);
    }
    bool ParseArgs(const CompilerInstance &CI, const std::vector<std::string> &args) override {
        return true;
    }
};

static FrontendPluginRegistry::Add<SaveFuncParmsAction> X("save-func-parms", "print function names");