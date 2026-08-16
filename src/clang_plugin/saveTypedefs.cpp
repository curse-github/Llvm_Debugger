#include <fstream>

#include "clangHelpers.h"
//#define DEBUG

class SaveTypedefsConsumer : public ASTConsumer {
    std::set<std::string> ParsedTemplates;
    std::fstream* TypedefsOut;
    public:
    SaveTypedefsConsumer(std::set<std::string> ParsedTemplates)
        : ParsedTemplates(ParsedTemplates) {
            TypedefsOut = new std::fstream();
            TypedefsOut->open("tmp/typedefs.csv", std::ios::app);
        }
    ~SaveTypedefsConsumer() {
        TypedefsOut->close();
        delete TypedefsOut;
        TypedefsOut = nullptr;
    }
    void HandleEnumDecl(EnumDecl* ED, std::string nameOverride="") {
        std::string name = ED->getNameAsString();
        if (name.size() == 0)
            name = nameOverride;
        if (name.size() == 0)
            return;
        unsigned int n = 0;
        for (EnumDecl::enumerator_iterator i = ED->enumerator_begin(), e = ED->enumerator_end(); i != e; ++i) n++;
        if (n == 0) return;
        if (knownEnums.count(name) > 0) return;
#ifdef DEBUG
        std::cout << "found enum \"" << name << "\"\n";
#endif
        knownEnums[name] = true;
        *TypedefsOut << "enum," << typeToString(ED->getIntegerType()) << ',' << name << ',' << n << ',';
        for (EnumDecl::enumerator_iterator i = ED->enumerator_begin(), e = ED->enumerator_end(); i != e; ++i) {
            EnumConstantDecl *EcD = *i;
            *TypedefsOut << EcD->getInitVal().getExtValue() << ',' << EcD->getNameAsString() << ',';
        }
        *TypedefsOut << '\n';
    }
    void HandleStructDecl(RecordDecl* RD, std::string nameOverride="") {
        std::string name = RD->getNameAsString();
        if (name.size() == 0)// this will usually be 0
            name = nameOverride;
        if (name.size() == 0)
            return;
        unsigned int n = 0;
        for (RecordDecl::field_iterator i = RD->field_begin(), e = RD->field_end(); i != e; ++i) n++;
        if (n == 0) return;
        if (knownStructs.count(name) > 0) return;
#ifdef DEBUG
        std::cout << "found struct \"" << name << "\"\n";
#endif
        knownStructs[name] = true;
        *TypedefsOut << "struct," << name << ',' << n << ',';
        for (RecordDecl::field_iterator i = RD->field_begin(), e = RD->field_end(); i != e; ++i) {
            FieldDecl *FD = *i;
            *TypedefsOut << typeToString(FD->getType()) << ',' << FD->getNameAsString() << ',';
        }
        *TypedefsOut << '\n';
    }

    void HandleUnionDecl(RecordDecl* RD, std::string nameOverride="") {
        std::string name = RD->getNameAsString();
        if (name.size() == 0)// this will usually be 0
            name = nameOverride;
        if (name.size() == 0)
            return;
        unsigned int n = 0;
        for (RecordDecl::field_iterator i = RD->field_begin(), e = RD->field_end(); i != e; ++i) n++;
        if (n == 0) return;
        if (knownStructs.count(name) > 0) return;
#ifdef DEBUG
        std::cout << "found union \"" << name << "\"\n";
#endif
        knownUnions[name] = true;
        *TypedefsOut << "union," << name << ',' << n << ',';
        for (RecordDecl::field_iterator i = RD->field_begin(), e = RD->field_end(); i != e; ++i) {
            FieldDecl *FD = *i;
            *TypedefsOut << typeToString(FD->getType()) << ',' << FD->getNameAsString() << ',';
        }
        *TypedefsOut << '\n';
    }
    void handleDecl(Decl* D) {
        if (TypedefDecl *TdD = dyn_cast<TypedefDecl>(D)) {
            std::string typeStr = TdD->getUnderlyingType().getAsString();
            if ((typeStr.find("<") != std::string::npos))
                return;
            if (typeStr.starts_with("struct ")) {
                if (TagDecl* tmp = TdD->getAnonDeclWithTypedefName())
                    if (RecordDecl* RD = dyn_cast<RecordDecl>(tmp))
                        HandleStructDecl(RD, typeStr.substr(7z, typeStr.size()-7z));
            } else if (typeStr.starts_with("enum ")) {
                if (TagDecl* tmp = TdD->getAnonDeclWithTypedefName())
                    if (EnumDecl* ED = dyn_cast<EnumDecl>(tmp))
                        HandleEnumDecl(ED, typeStr.substr(5z, typeStr.size()-5z));
            } else if (typeStr.starts_with("union ")) {
                if (TagDecl* tmp = TdD->getAnonDeclWithTypedefName())
                    if (RecordDecl* RD = dyn_cast<RecordDecl>(tmp))
                        HandleUnionDecl(RD, typeStr.substr(6z, typeStr.size()-6z));
            }
        } else if (EnumDecl *ED = dyn_cast<EnumDecl>(D))
            HandleEnumDecl(ED);
        else if (RecordDecl *RD = dyn_cast<RecordDecl>(D)) {
            std::string name = RD->getNameAsString();
            if (name.size() > 0)
                HandleStructDecl(RD);
        }
    }
    bool HandleTopLevelDecl(DeclGroupRef DG) override {
        for (DeclGroupRef::iterator i = DG.begin(), e = DG.end(); i != e; ++i)
            handleDecl(*i);
        return true;
    }
};
class SaveTypedefsAction : public PluginASTAction {
    std::set<std::string> ParsedTemplates;
    protected:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, llvm::StringRef) override {
        return std::make_unique<SaveTypedefsConsumer>(ParsedTemplates);
    }
    bool ParseArgs(const CompilerInstance &CI, const std::vector<std::string> &args) override {
        return true;
    }
};

static FrontendPluginRegistry::Add<SaveTypedefsAction> X("save-typedefs", "print function names");