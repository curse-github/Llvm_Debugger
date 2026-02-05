#include "../include/llvmHelpers.h"

llvm::Module* Module = nullptr;
llvm::LLVMContext* Context = nullptr;

llvm::Type* ptr_t = nullptr;
llvm::Type* char_t = nullptr;

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

llvm::GlobalVariable* createGlobalString(std::string str) {
    // create string to be used
    llvm::GlobalVariable* gStr = new llvm::GlobalVariable(*Module, llvm::ArrayType::get(char_t, static_cast<unsigned int>(str.size())+1u), true, llvm::GlobalValue::PrivateLinkage, 0, "str");
    gStr->setInitializer(llvm::ConstantDataArray::getString(*Context, str, true));
    return gStr;
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

std::string getTypeString(llvm::Type* ty) {
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
    else
        return "unknown";
    
}
llvm::GlobalVariable* unknownStr = nullptr;
llvm::CallInst* tryPrintValue(llvm::Value* val, llvm::BasicBlock::iterator beforeInstr) {
    if (unknownStr == nullptr)
        unknownStr = createGlobalString("unknown");
    llvm::Type* ty = val->getType();
    if (ty->isIntegerTy()) {
        const unsigned int bitWidth = ty->getIntegerBitWidth();
        switch(bitWidth) {
            case 8:
                return doCall(printChar, val, beforeInstr);
            case 32:
                return doCall(printUInt, val, beforeInstr);
            case 64:
                return doCall(printUInt64, val, beforeInstr);
        }
    } else if (ty->isFloatingPointTy()) {
        if (ty->isFloatTy())
            return doCall(printFloat, val, beforeInstr);
        else if (ty->isDoubleTy())
            return doCall(printDouble, val, beforeInstr);
    }
    return doCall(printStr, unknownStr, beforeInstr);
}