#include "llvmHelpers.h"

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

std::string valueToString(llvm::Value* inst) {
    std::string str;
    llvm::raw_string_ostream rso(str);
    inst->print(rso);
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

llvm::GlobalVariable* createGlobalString(std::string str, std::string varName) {
    llvm::GlobalVariable* gStr = new llvm::GlobalVariable(*Module, llvm::ArrayType::get(i8_t, static_cast<unsigned int>(str.size())+1u), true, llvm::GlobalValue::LinkageTypes::ExternalLinkage, 0, varName);
    gStr->setInitializer(llvm::ConstantDataArray::getString(*Context, str, true));
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
    else
        return "unknown";
}
std::string getTypeAsString(llvm::Value* val, llvm::Function* topLevelFunction) {
    llvm::Type* ty = val->getType();
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
        return attemptFindPointerType(val, topLevelFunction);
    else if (ty->isVoidTy())
        return "void";
    else
        return "unknown";
    
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
std::string attemptFindPointerType(llvm::Value* val, llvm::Function* topLevelFunction) {
    std::vector<std::string> possible;
    for(llvm::User* user : val->users()) {
        if (llvm::dyn_cast_or_null<llvm::Instruction>(user)) {
            // if it used in a load instruction, the type is a pointer to the type load instruction
            if (llvm::dyn_cast_or_null<llvm::LoadInst>(user))
                possible.push_back(getTypeAsString(user, topLevelFunction) + "*");
            // if it used in a store instruction, the type may be found
            else if (llvm::dyn_cast_or_null<llvm::StoreInst>(user)) {
                llvm::StoreInst* store = llvm::dyn_cast_or_null<llvm::StoreInst>(user);
                if (store->getOperand(1) == val) {
                    // if its value is being set, its type is the value of the first operand
                    if (store->getOperand(0)->getType()->isPointerTy())
                        continue;
                    // if it is being set to the value of another pointer it is probably better to find the type somewhere else
                    possible.push_back(getTypeAsString(store->getOperand(0), topLevelFunction) + "*");
                }// else
                    // if it is being used to set, you technically could get from the value of the second operand
                    // but skipping for now
            // if it used in a getelementptr instruction, the type is the same as the getelementptr instruction
            } else if (llvm::dyn_cast_or_null<llvm::GetElementPtrInst>(user))
                possible.push_back(attemptFindPointerType(user, topLevelFunction));
            // if it used in a call instruction, get the type from how the argument is used in that function
            else if (llvm::dyn_cast_or_null<llvm::CallInst>(user)) {
                llvm::CallInst* call = llvm::dyn_cast<llvm::CallInst>(user);
                // help avoid recursion
                if ((topLevelFunction != nullptr) && (call->getCalledFunction() == topLevelFunction))
                    continue;
                if (llvm::dyn_cast_or_null<llvm::Constant>(call->getCalledFunction()) == nullptr)
                    continue;
                unsigned int numArgs = std::min(user->getNumOperands(), static_cast<unsigned int>(call->getCalledFunction()->arg_size()));
                for (unsigned int i = 0; i < numArgs; i++)
                    if (user->getOperand(i) == val) {
                        possible.push_back(attemptFindPointerType(call->getCalledFunction()->getArg(i), call->getFunction()));
                        break;
                    }
            }
        }
    }
    for (size_t i = 0; i < possible.size(); i++) {
        if (possible[i].starts_with("void*"))
            continue;
        return possible[i];
    }
    return "void*";
}
llvm::GlobalVariable* unknownStr = nullptr;
void tryPrintValue(llvm::Value* val, llvm::BasicBlock::iterator beforeInst) {
    if (unknownStr == nullptr)
        unknownStr = createGlobalString("unknown");
    llvm::Type* ty = val->getType();
    if (ty->isIntegerTy()) {
        const unsigned int bitWidth = ty->getIntegerBitWidth();
        switch(bitWidth) {
            case 8:
                doCall(printChar, val, beforeInst);
                return;
            case 32:
                doCall(printUInt, val, beforeInst);
                return;
            case 64:
                doCall(printUInt64, val, beforeInst);
                return;
        }
    } else if (ty->isFloatingPointTy()) {
        if (ty->isFloatTy()) {
            doCall(printFloat, val, beforeInst);
            return;
        } else if (ty->isDoubleTy()) {
            doCall(printDouble, val, beforeInst);
            return;
        }
    } else if (ty->isPointerTy()) {
        std::string typeName = attemptFindPointerType(val);
        if (typeName == "char[]") {
            doCall(printChar, '\"', beforeInst);
            doCall(printStr, val, beforeInst);
            doCall(printChar, '\"', beforeInst);
        } else
            doCall(printStr, createGlobalString("{value of type " + typeName + '}'), beforeInst);
        return;
    }
    doCall(printStr, unknownStr, beforeInst);
}