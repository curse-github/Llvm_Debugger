#ifndef __CLANG_HELPERS
#define __CLANG_HELPERS

#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Sema/Sema.h"
using namespace clang;

#include <iostream>
#include <string>
#include <map>

extern std::map<std::string, bool> knownStructs;
extern std::map<std::string, bool> knownEnums;
extern std::map<std::string, bool> knownClasses;

std::string fixTypeName(std::string inputT, bool allowVoid=false);
std::string typeToString(const QualType& T, bool allowVoid=false);
void printFunctionSig(const FunctionDecl* FD);

#endif // __CLANG_HELPERS