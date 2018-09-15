#include "../include/Piet.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Verifier.h>

//#include <llvm/IR/BasicBlock.h>
//#include <llvm/IR/LLVMContext.h>
//#include <llvm/IR/Type.h>

//#include <llvm/Support/FileSystem.h>
//#include <llvm/Target/TargetMachine.h>
//#include <llvm/IR/LegacyPassManager.h>
//#include <llvm/ADT/STLExtras.h>
//#include <llvm/IR/Module.h>
//#include <llvm/Support/TargetSelect.h>
//#include <llvm/Support/TargetRegistry.h>
//#include <llvm/IR/GlobalVariable.h>

#include <iostream>
#include <unordered_map>

using namespace std;
using namespace llvm;


namespace Piet {
    const string RUNTIME_PUSH = "push";

    void registerPietGlobals(unique_ptr<Module> module) {
        // Register push.
        std::vector<Type *> pushArgs(1, Type::getInt32Ty(module->getContext()));
        FunctionType *pushType = FunctionType::get(Type::getVoidTy(module->getContext()), pushArgs, false);
        Function::Create(pushType, Function::InternalLinkage, RUNTIME_PUSH, module.get());
    }

    void Translator::translateToExecutable(string filename) {
        LLVMContext Context;
        IRBuilder<> Builder(Context);
        unique_ptr<Module> PietModule = llvm::make_unique<Module>("piet", Context);
        registerPietGlobals(move(PietModule));

        Parse::GraphStep *step;

        while ((step = graph->walk()) != nullptr) {
            cout << "previous color: " << step->previous->getColor() << "; previous size: " << step->previous->getSize() << endl;
            cout << "next color: " << step->next->getColor() << "; next size: " << step->next->getSize() << endl;

            // Determine operation from step.
            auto transition = ColorTransition::determineTransition(step->previous, step->next);
            if (transition == nullptr) {
                // No operation.
                cout << "No operation" << endl;
            } else {
                assert(transition->getHueChange() < operationTable.size());
                assert(transition->getLightnessChange() < operationTable[transition->getHueChange()].size());
                string operation = operationTable[transition->getHueChange()][transition->getLightnessChange()];
                assert(operation != "");
                cout << "Operation: " << operationTable[transition->getHueChange()][transition->getLightnessChange()] << endl;
            }


            // End sequence if terminal node.
            if (step->next->isTerminal()) {
                cout << "Terminate!" << endl;
            }

            cout << endl;
        }

        auto mainFunction = cast<Function>(PietModule->getOrInsertFunction("main",
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

        auto testFunc = cast<Function>(PietModule->getOrInsertFunction("test",
                                                                        IntegerType::getInt32Ty(Context),
                                                                        IntegerType::getInt32Ty(Context),
                                                                        IntegerType::getInt8Ty(Context)));
        {
            Function::arg_iterator args = testFunc->arg_begin();
            args[0].setName("argc");
            args[1].setName("argv");
        }

        Builder.CreateRet(ConstantInt::get(Context, APInt(32, 0)));


        verifyModule(*(PietModule.get()), &errs());
        PietModule->print(errs(), nullptr);
    }
}
