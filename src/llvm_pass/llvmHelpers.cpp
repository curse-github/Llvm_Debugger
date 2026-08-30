#include "llvmHelpers.h"

std::vector<llvm::Function*> visitedFunctions_global;

llvm::Module* Module = nullptr;
llvm::LLVMContext* Context = nullptr;

llvm::Type* void_t = nullptr;// void
llvm::Type* ptr_t = nullptr;// void*
llvm::Type* i1_t = nullptr;// bool
llvm::Type* i8_t = nullptr;// char
llvm::Type* i16_t = nullptr;// short
llvm::Type* i32_t = nullptr;// int
llvm::Type* i64_t = nullptr;// long
llvm::Type* f16_t = nullptr;// half
llvm::Type* f32_t = nullptr;// float
llvm::Type* f64_t = nullptr;// double

llvm::Function* printChar = nullptr;
llvm::Function* printStr = nullptr;
//llvm::Function* printBool = nullptr;
llvm::Function* printUInt = nullptr;
llvm::Function* printUInt64 = nullptr;
llvm::Function* printFloat = nullptr;
llvm::Function* printDouble = nullptr;
llvm::Function* printlnChar = nullptr;

void populateGlobals(llvm::Module& _Module) {
    if (!Module)
        Module = &_Module;
    populateGlobals();
}
void populateGlobals(llvm::Function& F) {
    if (!Module)
        Module = F.getParent();
    populateGlobals();
}
void populateGlobals() {
    if (Context == nullptr) {
        Context = &Module->getContext();
        void_t = llvm::Type::getVoidTy(*Context);
        ptr_t = llvm::PointerType::get(*Context, 0);
        i1_t = llvm::Type::getInt1Ty(*Context);
        i8_t = llvm::Type::getInt8Ty(*Context);
        i32_t = llvm::Type::getInt16Ty(*Context);
        i32_t = llvm::Type::getInt32Ty(*Context);
        i64_t = llvm::Type::getInt64Ty(*Context);
        f16_t = llvm::Type::getHalfTy(*Context);
        f32_t = llvm::Type::getFloatTy(*Context);
        f64_t = llvm::Type::getDoubleTy(*Context);
    }
}
void populateStdLib(llvm::Function& F) {
    if (!Module)
        populateGlobals(F);
    if (!printChar) {
        printChar = Module->getFunction("printChar");
        printStr = Module->getFunction("printStr");
        //printBool = Module->getFunction("printBool");
        printUInt = Module->getFunction("printUInt");
        printUInt64 = Module->getFunction("printUInt64");
        printFloat = Module->getFunction("printFloat");
        printDouble = Module->getFunction("printDouble");
        printlnChar = Module->getFunction("printlnChar");
    }
}

std::string valueToString(llvm::Value* val) {
    std::string str;
    llvm::raw_string_ostream rso(str);
    val->print(rso);
    return str;
}
std::string typeToString(llvm::Type* ty) {
    std::string str;
    llvm::raw_string_ostream rso(str);
    ty->print(rso);
    return str;
}
void printFuncSig(const llvm::Function& F) {
    std::cout << llvm::demangle(F.getName().str()) << "(";
    const unsigned int arg_size = F.arg_size();
    for(unsigned int i = 0; i < arg_size; i++) {
        llvm::Argument* arg = F.getArg(i);
        std::cout << ((i != 0) ? ", " : "") << getTypeAsString(arg);
    }
    std::cout << ")\n";
}

std::map<std::string, llvm::GlobalVariable*> globals;
llvm::GlobalVariable* createGlobalString(std::string str, std::string varName) {
    if (globals.count(str) > 0)
        return globals[str];
    llvm::GlobalVariable* gStr = new llvm::GlobalVariable(*Module, llvm::ArrayType::get(i8_t, static_cast<unsigned int>(str.size())+1u), true, llvm::GlobalValue::LinkageTypes::ExternalLinkage, 0, varName);
    gStr->setInitializer(llvm::ConstantDataArray::getString(*Context, str, true));
    globals[str] = gStr;
    return gStr;
}
llvm::GlobalVariable* createGlobalPtrArray(llvm::ArrayRef<llvm::Constant*> vals, std::string varName) {
    llvm::ArrayType* T = llvm::ArrayType::get(ptr_t, static_cast<unsigned int>(vals.size()));
    llvm::GlobalVariable* gArr = new llvm::GlobalVariable(*Module, T, true, llvm::GlobalValue::LinkageTypes::ExternalLinkage, 0, varName);
    gArr->setInitializer(llvm::ConstantArray::get(T, vals));
    return gArr;
}
llvm::GlobalVariable* createGlobalInt(int val, std::string varName) {
    llvm::GlobalVariable* gInt = new llvm::GlobalVariable(*Module, i32_t, true, llvm::GlobalValue::LinkageTypes::ExternalLinkage, 0, varName);
    gInt->setInitializer(llvm::ConstantInt::get(i32_t, val));
    return gInt;
}
llvm::GlobalVariable* createGlobalIntArray(llvm::ArrayRef<llvm::Constant*> vals, std::string varName) {
    llvm::ArrayType* T = llvm::ArrayType::get(i32_t, static_cast<unsigned int>(vals.size()));
    llvm::GlobalVariable* gArr = new llvm::GlobalVariable(*Module, T, true, llvm::GlobalValue::LinkageTypes::ExternalLinkage, 0, varName);
    gArr->setInitializer(llvm::ConstantArray::get(T, vals));
    return gArr;
}
llvm::GlobalVariable* createGlobalCharArray(llvm::ArrayRef<llvm::Constant*> vals, std::string varName) {
    llvm::ArrayType* T = llvm::ArrayType::get(i8_t, static_cast<unsigned int>(vals.size()));
    llvm::GlobalVariable* gArr = new llvm::GlobalVariable(*Module, T, true, llvm::GlobalValue::LinkageTypes::ExternalLinkage, 0, varName);
    gArr->setInitializer(llvm::ConstantArray::get(T, vals));
    return gArr;
}
llvm::GlobalVariable* createGlobalBoolArray(llvm::ArrayRef<llvm::Constant*> vals, std::string varName) {
    llvm::ArrayType* T = llvm::ArrayType::get(i8_t, static_cast<unsigned int>(vals.size()));
    llvm::GlobalVariable* gArr = new llvm::GlobalVariable(*Module, T, true, llvm::GlobalValue::LinkageTypes::ExternalLinkage, 0, varName);
    gArr->setInitializer(llvm::ConstantArray::get(T, vals));
    return gArr;
}

llvm::CallInst* doCall(llvm::Function* f, llvm::Value* val, llvm::BasicBlock::iterator beforeInst) {
    // create print function call
    llvm::CallInst* printCall = llvm::CallInst::Create(f, { val }, "");
    printCall->setTailCall();
    printCall->insertBefore(beforeInst);
    return printCall;
}
llvm::CallInst* doCall(llvm::Function* f, char chr, llvm::BasicBlock::iterator beforeInst) {
    // create print function call
    llvm::CallInst* printCall = llvm::CallInst::Create(f, { llvm::ConstantInt::get(i8_t, chr, true) }, "");
    printCall->setTailCall();
    printCall->insertBefore(beforeInst);
    return printCall;
}

unsigned int structCount = 0;
std::vector<std::string> structNames;
std::map<llvm::Type*, unsigned int> structTypeToIndex;
std::map<std::string, unsigned int> structNameToIndex;
std::vector<llvm::StructType*> unnamedStructTypes;
unsigned int unionCount = 0;
std::vector<std::string> unionNames;
std::map<llvm::Type*, unsigned int> unionTypeToIndex;
std::map<std::string, unsigned int> unionNameToIndex;

std::string basicGetTypeAsString(llvm::Type* ty) {
    if (ty->isIntegerTy()) {
        const unsigned int bitWidth = ty->getIntegerBitWidth();
        switch(bitWidth) {
            case 1:
                return "bool";
            case 8:
                return "char";
            case 16:
                return "short";
            case 32:
                return "int";
            case 64:
                return "long";
            default:
                return "unknown";
        }
    } else if (ty->isFloatingPointTy()) {
        if (ty->isHalfTy())
            return "half";
        else if (ty->isFloatTy())
            return "float";
        else if (ty->isDoubleTy())
            return "double";
        else
            return "unknown";
    } else if (ty->isPointerTy())
        return "void*";
    else if (ty->isVoidTy())
        return "void";
    else if (ty->isStructTy()) {
        if (structTypeToIndex.count(ty) > 0)
            return structNames[structTypeToIndex[ty]];
        std::string name = "unknown" + std::to_string(unnamedStructTypes.size());
        unnamedStructTypes.push_back(llvm::dyn_cast<llvm::StructType>(ty));
        structTypeToIndex[ty] = structCount;
        structNameToIndex[name] = structCount;
        structCount++;
        structNames.push_back(name);
        return name;
    } else if (ty->isArrayTy()) {
        llvm::ArrayType* aty = (llvm::ArrayType*)ty;
        return basicGetTypeAsString(aty->getElementType()) + '[' + std::to_string(aty->getNumElements()) + ']';
    } else
        return "unknown";
}
std::string attemptFindPointerType(llvm::Value* val, std::vector<llvm::Function*>& visitedFunctions=visitedFunctions_global, unsigned int depth=0, std::string indent="    ");
std::map<llvm::Value*, std::string> determinedTypes;
std::string getTypeAsString(llvm::Value* val, std::vector<llvm::Function*>& visitedFunctions, unsigned int depth, std::string indent) {
    if (determinedTypes.count(val)!=0)
        return determinedTypes[val];
    llvm::Type* ty = val->getType();
    std::string typeStr = "unknown";
    if (ty->isPointerTy()) {
        if (llvm::dyn_cast_or_null<llvm::Constant>(val))
            typeStr = "void*";// is essentially a nullptr or void*
        else
            typeStr = attemptFindPointerType(val, visitedFunctions, depth, indent+"    ");
    } else
        typeStr = basicGetTypeAsString(ty);
    determinedTypes[val] = typeStr;
    return typeStr;
}
int getTypeBitWidth(llvm::Type* ty) {
    if (ty->isIntegerTy()) {
        const unsigned int bitWidth = ty->getIntegerBitWidth();
        switch(bitWidth) {
            case 1:
            case 8:
            case 16:
            case 32:
            case 64:
                return bitWidth;
            default:
                return -1;
        }
    } else if (ty->isFloatingPointTy()) {
        if (ty->isHalfTy())
            return 16;
        else if (ty->isFloatTy())
            return 32;
        else if (ty->isDoubleTy())
            return 64;
        else
            return -1;
    } else if (ty->isPointerTy())
        return 64;
    else
        return -1;
}
std::string attemptFindPointerType(llvm::Value* val, std::vector<llvm::Function*>& visitedFunctions, unsigned int depth, std::string indent) {
    if (depth>1000) {
        //std::cout << "reached max depth\n";
        return "void*";
    }
    std::vector<std::string> possible;
    for(llvm::User* user : val->users()) {
        if (llvm::dyn_cast_or_null<llvm::Instruction>(user) != nullptr) {
            // if it used in a load instruction, the type is a pointer to the type load instruction
            if (llvm::dyn_cast_or_null<llvm::LoadInst>(user) != nullptr)
                possible.push_back(getTypeAsString(user, visitedFunctions, depth+1, indent+"    ") + "*");
            // if it used in a store instruction, the type may be found
            else if (llvm::dyn_cast_or_null<llvm::StoreInst>(user) != nullptr) {
                llvm::StoreInst* store = llvm::dyn_cast<llvm::StoreInst>(user);
                if (store->getOperand(1) == val) {
                    // if its value is being set, its type is the value of the first operand
                    possible.push_back(getTypeAsString(store->getOperand(0), visitedFunctions, depth+1, indent+"    ") + "*");
                } else {
                    std::string tmp = getTypeAsString(store->getOperand(1), visitedFunctions, depth+1, indent+"    ");
                    if (tmp[tmp.size()-1] == '*') {
                        if (tmp == "void*")
                            possible.push_back("void*");
                        else
                            possible.push_back(tmp.substr(0, tmp.size()-1));
                    }
                }
            // if it used in a call instruction, get the type from how the argument is used in that function
            } else if (llvm::dyn_cast_or_null<llvm::CallInst>(user) != nullptr) {
                llvm::CallInst* call = llvm::dyn_cast<llvm::CallInst>(user);
                // help avoid recursion
                llvm::Function* func = call->getCalledFunction();
                if (func != nullptr) {
                    // if function is only declarations no analysis can be done on its body
                    if (llvm::dyn_cast_or_null<llvm::GlobalValue>(func) == nullptr)
                        continue;
                    if (func->isDeclaration())
                        continue;
                    // function cannot be checked already
                    if (std::find(visitedFunctions.begin(), visitedFunctions.end(), func) != visitedFunctions.end())
                        continue;
                    unsigned int numArgs = std::min(user->getNumOperands(), static_cast<unsigned int>(func->arg_size()));
                    for (unsigned int i = 0; i < numArgs; i++)
                        if (user->getOperand(i) == val) {
                            visitedFunctions.push_back(call->getFunction());
                            possible.push_back(getTypeAsString(func->getArg(i), visitedFunctions, depth+1, indent+"    "));
                            break;
                        }
                } else {
                    llvm::Value* called = call->getCalledOperand();
                    if (called == val)
                        return "f_ptr";
                }
            // if it used in a getelementptr instruction, the type is the same as the getelementptr instruction
            } else if (llvm::dyn_cast_or_null<llvm::GetElementPtrInst>(user) != nullptr) {
                llvm::GetElementPtrInst* gepi = llvm::dyn_cast<llvm::GetElementPtrInst>(user);
                if (gepi->isInBounds()) {
                    const std::string tmp = basicGetTypeAsString(gepi->getSourceElementType());
                    if (tmp != "char")
                        possible.push_back(tmp + '*');
                    else
                        possible.push_back(getTypeAsString(gepi, visitedFunctions, depth+1, indent+"    "));
                } else
                    possible.push_back(getTypeAsString(gepi, visitedFunctions, depth+1, indent+"    "));
            // if it used in a phi instruction, the type is the same as the phi instruction
            } else if (llvm::dyn_cast_or_null<llvm::PHINode>(user) != nullptr) {
                possible.push_back(getTypeAsString(user, visitedFunctions, depth+1, indent+"    "));
            } else {
                //std::cout << valueToString(user) << '\n';
            }
        }
    }
    if (depth == 0)
        visitedFunctions.clear();
    for (size_t i = 0; i < possible.size(); i++) {
        if (possible[i].starts_with("void*"))
            continue;
        return possible[i];
    }
    return "void*";
}