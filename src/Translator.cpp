#include "../include/Piet.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/TargetRegistry.h>

#include <iostream>
#include <unordered_map>

using namespace std;
using namespace llvm;


namespace Piet {
    Function *push;
    Function *duplicate;
    Function *outChar;

    void registerPietGlobals(Module &module) {
        // Register push.
        std::vector<Type *> pushArgs(1, Type::getInt32Ty(module.getContext()));
        FunctionType *pushType = FunctionType::get(Type::getVoidTy(module.getContext()), pushArgs, false);
        push = Function::Create(pushType, Function::ExternalLinkage, "mondriaan_runtime_push", &module);

        // Register out(char).
        FunctionType *outCharType = FunctionType::get(Type::getVoidTy(module.getContext()), vector<Type *>(), false);
        outChar = Function::Create(outCharType, Function::ExternalLinkage, "mondriaan_runtime_out_char", &module);

        // Register duplicate.
        FunctionType *duplicateType = FunctionType::get(Type::getVoidTy(module.getContext()), vector<Type *>(), false);
        duplicate = Function::Create(duplicateType, Function::ExternalLinkage, "mondriaan_runtime_duplicate", &module);
    }

    void Translator::translateIRToExecutable(string objectFilename, Module &module) {
        // Initialize the target registry etc.
        InitializeAllTargetInfos();
        InitializeAllTargets();
        InitializeAllTargetMCs();
        InitializeAllAsmParsers();
        InitializeAllAsmPrinters();

        auto targetTriple = sys::getDefaultTargetTriple();
        module.setTargetTriple(targetTriple);

        std::string error;
        auto target = TargetRegistry::lookupTarget(targetTriple, error);

        // Print an error and exit if we couldn't find the requested target.
        // This generally occurs if we've forgotten to initialise the
        // TargetRegistry or we have a bogus target triple.
        if (!target) {
            errs() << error;
            exit(1);
        }

        auto cpu = "generic";
        auto features = "";

        TargetOptions opt;
        auto rm = Optional<Reloc::Model>();
        auto targetMachine = target->createTargetMachine(targetTriple, cpu, features, opt, rm);

        module.setDataLayout(targetMachine->createDataLayout());

        std::error_code ec;
        raw_fd_ostream dest(objectFilename, ec, sys::fs::F_None);

        if (ec) {
            errs() << "Could not open file: " << ec.message();
            exit(1);
        }

        legacy::PassManager pass;

        if (targetMachine->addPassesToEmitFile(pass, dest, TargetMachine::CGFT_ObjectFile)) {
            errs() << "the target machine can't emit a file of this type";
            exit(1);
        }

        pass.run(module);
        dest.flush();

        // Link object file with runtime library and set entry point:
        // LIBRARY_PATH=<runtimelibdir> clang++ <objectfile> -lMondriaanRuntime -o <execfile>
    }

    void Translator::translateToExecutable(string filename) {
        LLVMContext Context;
        IRBuilder<> Builder(Context);
        Module PietModule("piet", Context);
        registerPietGlobals(PietModule);

        auto mainFunction = cast<Function>(PietModule.getOrInsertFunction("main",
                                                                        IntegerType::getInt32Ty(Context),
                                                                        IntegerType::getInt32Ty(Context),
                                                                        IntegerType::getInt8Ty(Context)));
        {
            Function::arg_iterator args = mainFunction->arg_begin();
            args[0].setName("argc");
            args[1].setName("argv");
        }

        BasicBlock *BB = BasicBlock::Create(Context, "entry", mainFunction);
        Builder.SetInsertPoint(BB);

        Parse::GraphStep *step;
        while ((step = graph->walk()) != nullptr) {
            cout << "previous color: " << step->previous->getColor() << "; previous size: " << step->previous->getSize() << endl;
            cout << "current color: " << step->current->getColor() << "; current size: " << step->current->getSize() << endl;

            // Determine operation from step.
            auto transition = ColorTransition::determineTransition(step->previous, step->current);
            if (transition == nullptr) {
                // No operation.
                cout << "No operation" << endl;
            } else {
                assert(transition->getHueChange() < operationTable.size());
                assert(transition->getLightnessChange() < operationTable[transition->getHueChange()].size());
                string operation = operationTable[transition->getHueChange()][transition->getLightnessChange()];
                assert(!operation.empty());

                if (operation == OP_PUSH) {
                    vector<Value *> pushArgs;
                    pushArgs.push_back(ConstantInt::get(Type::getInt32Ty(Context), APInt(32, step->previous->getSize())));
                    Builder.CreateCall(push, pushArgs);
                } else if (operation == OP_OUT_CHAR) {
                    Builder.CreateCall(outChar);
                } else if (operation == OP_DUPLICATE) {
                    Builder.CreateCall(duplicate);
                } else {
                    cout << "Yet unsupported operation: " << operationTable[transition->getHueChange()][transition->getLightnessChange()] << endl;
                }
            }

            // End sequence if terminal node.
            if (step->current->isTerminal()) {
                cout << "Terminate!" << endl;
            }

            cout << endl;
        }

        Builder.CreateRet(ConstantInt::get(Context, APInt(32, 0)));

        if (verifyModule(PietModule, &errs())) {
            exit(1);
        }
        PietModule.print(errs(), nullptr);

        translateIRToExecutable(filename + ".o", PietModule);
    }
}
