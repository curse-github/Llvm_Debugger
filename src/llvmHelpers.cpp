#include "llvmHelpers.h"

llvm::Module* Module = nullptr;
llvm::LLVMContext* Context = nullptr;

llvm::Type* ptr_t = nullptr;
llvm::Type* char_t = nullptr;
llvm::Type* i32_t = nullptr;

llvm::Function* printChar = nullptr;
llvm::Function* printStr = nullptr;
//llvm::Function* printBool = nullptr;
llvm::Function* printUInt = nullptr;
llvm::Function* printUInt64 = nullptr;
llvm::Function* printFloat = nullptr;
llvm::Function* printDouble = nullptr;
llvm::Function* printlnChar = nullptr;

void populateGlobals(llvm::Function& F) {
    if (!Module) {
        Module = F.getParent();
        Context = &Module->getContext();

        ptr_t = llvm::PointerType::get(*Context, 0);
        char_t = llvm::Type::getInt8Ty(*Context);
        i32_t = llvm::Type::getInt32Ty(*Context);
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

llvm::GlobalVariable* createGlobalString(std::string str, std::string varName) {
    llvm::GlobalVariable* gStr = new llvm::GlobalVariable(*Module, llvm::ArrayType::get(char_t, static_cast<unsigned int>(str.size())+1u), true, llvm::GlobalValue::LinkageTypes::ExternalLinkage, 0, varName);
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
    llvm::CallInst* printCall = llvm::CallInst::Create(f, { llvm::ConstantInt::get(char_t, chr, true) }, "");
    printCall->setTailCall();
    printCall->insertBefore(beforeInst);
    return printCall;
}

std::string getTypeAsString(llvm::Value* val) {
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
        return attemptFindPointerType(val);
    else
        return "unknown";
    
}
//#include <iostream>
std::string valueToString(llvm::Value* inst) {
    std::string str;
    llvm::raw_string_ostream rso(str);
    inst->print(rso);
    return str;
}
std::string attemptFindPointerType(llvm::Value* val, bool isArray) {
    for(llvm::User* user : val->users()) {
        if (llvm::dyn_cast_or_null<llvm::Instruction>(user)) {
            // if it used in a load instruction, the type is a pointer to the type load instruction
            if (llvm::dyn_cast_or_null<llvm::LoadInst>(user)) {
                return getTypeAsString(user) + (isArray?"[]":"*");
            // if it used in a getelementptr instruction, the type is the same as the getelementptr instruction
            } else if (llvm::dyn_cast_or_null<llvm::GetElementPtrInst>(user)) {
                return attemptFindPointerType(user, true);
            // if it used in a call instruction, get the type from how the argument is used in that function
            } else if (llvm::dyn_cast_or_null<llvm::CallInst>(user)) {
                llvm::CallInst* call = llvm::dyn_cast<llvm::CallInst>(user);
                for (unsigned int i = 0; i < user->getNumOperands(); i++)
                    if (user->getOperand(i) == val)
                        return attemptFindPointerType(call->getCalledFunction()->getArg(i));
                return isArray?"void[]":"void*";
            }
        }
    }
    //std::cout << "did not find known pointer use.\n";
    return isArray?"void[]":"void*";
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