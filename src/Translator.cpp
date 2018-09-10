#include "../include/Piet.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Support/raw_ostream.h>

//#include <llvm/IR/Function.h>
//#include <llvm/IR/IRBuilder.h>
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
//#include <llvm/IR/Verifier.h>
//#include <llvm/IR/GlobalVariable.h>

#include <iostream>

using namespace std;
using namespace llvm;

namespace Piet {

    void Translator::translateToExecutable(string filename) {
        LLVMContext Context;
        IRBuilder<> Builder(Context);
        unique_ptr<llvm::Module> PietModule = llvm::make_unique<Module>("piet", Context);


        Parse::GraphStep *step;

        while ((step = graph->walk()) != nullptr) {
            // Determine operation from step.
            // End sequence if terminal node.

            cout << "previous color: " << step->previous->getColor() << "; previous size: " << step->previous->getSize() << endl;
            cout << "next color: " << step->next->getColor() << "; next size: " << step->next->getSize() << endl;
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


        PietModule->print(errs(), nullptr);
    }
}
