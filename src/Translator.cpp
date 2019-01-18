#include "../include/Piet.h"

#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>

#include <iostream>
#include <unordered_map>

using namespace std;
using namespace llvm;

namespace Piet {
Function *push;
Function *duplicate;
Function *outChar;
Function *outNumber;
Function *pointerBranch;
Function *inNumber;

void Translator::registerPietGlobals() {
  // Register push.
  std::vector<Type *> pushArgs(1, Type::getInt32Ty(context));
  FunctionType *pushType =
      FunctionType::get(Type::getVoidTy(context), pushArgs, false);
  push = Function::Create(pushType, Function::ExternalLinkage,
                          "mondriaan_runtime_push", &module);

  // Register out(char).
  FunctionType *outCharType =
      FunctionType::get(Type::getVoidTy(context), vector<Type *>(), false);
  outChar = Function::Create(outCharType, Function::ExternalLinkage,
                             "mondriaan_runtime_out_char", &module);

  // Register out(number).
  FunctionType *outNumberType =
      FunctionType::get(Type::getVoidTy(context), vector<Type *>(), false);
  outNumber = Function::Create(outNumberType, Function::ExternalLinkage,
                               "mondriaan_runtime_out_number", &module);

  // Register duplicate.
  FunctionType *duplicateType =
      FunctionType::get(Type::getVoidTy(context), vector<Type *>(), false);
  duplicate = Function::Create(duplicateType, Function::ExternalLinkage,
                               "mondriaan_runtime_duplicate", &module);

  // Register pointer.
  FunctionType *pointerType =
      FunctionType::get(Type::getInt32Ty(context), vector<Type *>(), false);
  pointerBranch = Function::Create(pointerType, Function::ExternalLinkage,
                                   "mondriaan_runtime_pointer", &module);

  // Register in(number);
  FunctionType *inNumberType =
      FunctionType::get(Type::getVoidTy(context), vector<Type *>(), false);
  inNumber = Function::Create(inNumberType, Function::ExternalLinkage,
                              "mondriaan_runtime_in_number", &module);
}

void Translator::translateIRToExecutable(string objectFilename) {
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
  auto targetMachine =
      target->createTargetMachine(targetTriple, cpu, features, opt, rm);

  module.setDataLayout(targetMachine->createDataLayout());

  std::error_code ec;
  raw_fd_ostream dest(objectFilename, ec, sys::fs::F_None);

  if (ec) {
    errs() << "Could not open file: " << ec.message();
    exit(1);
  }

  legacy::PassManager pass;

  if (targetMachine->addPassesToEmitFile(pass, dest,
                                         TargetMachine::CGFT_ObjectFile)) {
    errs() << "the target machine can't emit a file of this type";
    exit(1);
  }

  pass.run(module);
  dest.flush();

  // Link object file with runtime library and set entry point:
  // LIBRARY_PATH=<runtimelibdir> clang++ <objectfile> -lMondriaanRuntime -o
  // <execfile>
}

Function *Translator::translateBranch(Parse::GraphNode *node,
                                      DirectionPoint dp) {
  graph->restartWalk(node, dp);

  Function *openFunction = Function::Create(
      FunctionType::get(Type::getVoidTy(context), false),
      Function::PrivateLinkage, "next_mondriaan_sequence", &module);

  BasicBlock *openBlock =
      BasicBlock::Create(context, "mondriaan_seq", openFunction);
  builder.SetInsertPoint(openBlock);

  Parse::GraphStep *step;
  string sequenceID;

  // while (stack is not empty) OR (we can continue walking the graph)
  while ((step = graph->walk()) != nullptr) {
    if (sequenceID.empty()) {
      sequenceID = step->previous->getIdentifier();
    } else {
      sequenceID += "_" + step->previous->getIdentifier();
    }

    cout << "previous color: " << hex << step->previous->getColor() << dec
         << "; previous size: " << step->previous->getSize() << endl;
    cout << "current color: " << hex << step->current->getColor() << dec
         << "; current size: " << step->current->getSize() << endl;

    // Determine operation from step.
    auto transition = !step->skipTransition
                          ? ColorTransition::determineTransition(step->previous,
                                                                 step->current)
                          : nullptr;
    if (transition == nullptr) {
      // No operation.
      cout << "No operation" << endl;
    } else {
      assert(transition->getHueChange() < operationTable.size());
      assert(transition->getLightnessChange() <
             operationTable[transition->getHueChange()].size());
      string operation = operationTable[transition->getHueChange()]
                                       [transition->getLightnessChange()];
      assert(!operation.empty());

      if (operation == OP_PUSH) {
        vector<Value *> pushArgs;
        pushArgs.push_back(ConstantInt::get(
            Type::getInt32Ty(context), APInt(32, step->previous->getSize())));
        builder.CreateCall(push, pushArgs);
      } else if (operation == OP_OUT_CHAR) {
        builder.CreateCall(outChar);
      } else if (operation == OP_OUT_NUMBER) {
        builder.CreateCall(outNumber);
      } else if (operation == OP_DUPLICATE) {
        builder.CreateCall(duplicate);
      } else if (operation == OP_IN_NUMBER) {
        builder.CreateCall(inNumber);
      } else if (operation == OP_POINTER) {
        if (translatedBranches[sequenceID]) {
          openFunction->removeFromParent();
          return translatedBranches[sequenceID];
        }

        auto currentNode = graph->getCurrentNode();

        // End the current function.
        translatedBranches[sequenceID] = openFunction;
        openFunction->setName(sequenceID);

        // Call pointer in the entry block.
        Value *pointerVal = builder.CreateCall(pointerBranch, None, "pointer");

        // Create 4 new jump function prototypes each with their own block.
        DirectionPoint noTurn = graph->getCurrentDirection(),
                       singleTurn = incrementDirectionPointer(noTurn),
                       doubleTurn = incrementDirectionPointer(singleTurn),
                       tripleTurn = incrementDirectionPointer(doubleTurn);
        auto noTurnBranch = translateBranch(currentNode, noTurn),
             singleTurnBranch = translateBranch(currentNode, singleTurn),
             doubleTurnBranch = translateBranch(currentNode, doubleTurn),
             tripleTurnBranch = translateBranch(currentNode, tripleTurn);

        BasicBlock *noTurnJumpBlock = BasicBlock::Create(
            context, "mondriaan.pointer.jmp.0", openFunction);
        builder.SetInsertPoint(noTurnJumpBlock);
        builder.CreateCall(noTurnBranch);
        builder.CreateRetVoid();
        BasicBlock *singleTurnJumpBlock = BasicBlock::Create(
            context, "mondriaan.pointer.jmp.1", openFunction);
        builder.SetInsertPoint(singleTurnJumpBlock);
        builder.CreateCall(singleTurnBranch);
        builder.CreateRetVoid();
        BasicBlock *doubleTurnJumpBlock = BasicBlock::Create(
            context, "mondriaan.pointer.jmp.2", openFunction);
        builder.SetInsertPoint(doubleTurnJumpBlock);
        builder.CreateCall(doubleTurnBranch);
        builder.CreateRetVoid();
        BasicBlock *tripleTurnJumpBlock = BasicBlock::Create(
            context, "mondriaan.pointer.jmp.3", openFunction);
        builder.SetInsertPoint(tripleTurnJumpBlock);
        builder.CreateCall(tripleTurnBranch);
        builder.CreateRetVoid();

        // Create 3 blocks for the 4-way logic.
        BasicBlock *doubleTurnTestBlock = BasicBlock::Create(
            context, "mondriaan.pointer.test.2", openFunction);
        builder.SetInsertPoint(doubleTurnTestBlock);
        const auto twoVal = builder.getInt(APInt(32, 2));
        Value *twoCondition =
            builder.CreateICmp(CmpInst::ICMP_EQ, pointerVal, twoVal);
        builder.CreateCondBr(twoCondition, doubleTurnJumpBlock,
                             tripleTurnJumpBlock);
        BasicBlock *singleTurnTestBlock = BasicBlock::Create(
            context, "mondriaan.pointer.test.1", openFunction);
        builder.SetInsertPoint(singleTurnTestBlock);
        const auto oneVal = builder.getInt(APInt(32, 1));
        Value *oneCondition =
            builder.CreateICmp(CmpInst::ICMP_EQ, pointerVal, oneVal);
        builder.CreateCondBr(oneCondition, singleTurnJumpBlock,
                             doubleTurnTestBlock);
        BasicBlock *noTurnTestBlock = BasicBlock::Create(
            context, "mondriaan.pointer.test.0", openFunction);
        builder.SetInsertPoint(noTurnTestBlock);
        const auto zeroVal = builder.getInt(APInt(32, 0));
        Value *zeroCondition =
            builder.CreateICmp(CmpInst::ICMP_EQ, pointerVal, zeroVal);
        builder.CreateCondBr(zeroCondition, noTurnJumpBlock,
                             singleTurnTestBlock);

        // Jump to the start of logic blocks in the entry block.
        builder.SetInsertPoint(openBlock);
        builder.CreateBr(noTurnTestBlock);

        /**
         * entry:
         *  %pointer = call mondriaan_pointer
         *  br %mondriaan.pointer.test.0
         *
         * mondriaan.pointer.test.0:
         *  %cond0 = icmp eq i32 %pointer, %0
         *  br i1 %cond0, label %mondriaan.pointer.jmp.0, label
         * %mondriaan.pointer.test.1
         *
         * mondriaan.pointer.test.1:
         *  %cond1 = icmp eq i32 %pointer, %1
         *  br i1 %cond1, label %mondriaan.pointer.jmp.1, label
         * %mondriaan.pointer.test.2
         *
         * mondriaan.pointer.test.2:
         *  %cond2 = icmp eq i32 %pointer, %2
         *  br i1 %cond2, label %mondriaan.pointer.jmp.2, label
         * %mondriaan.pointer.jmp.3
         *
         * mondriaan.pointer.jmp.0:
         * mondriaan.pointer.jmp.1:
         * mondriaan.pointer.jmp.2:
         * mondriaan.pointer.jmp.3:
         */

        if (verifyFunction(*openFunction, &errs())) {
          exit(1);
        }
        return openFunction;
      } else {
        cout << "Yet unsupported operation: "
             << operationTable[transition->getHueChange()]
                              [transition->getLightnessChange()]
             << endl;
      }
    }

    // End sequence if terminal node.
    if (step->current->isTerminal()) {
      if (translatedBranches[sequenceID]) {
        openFunction->eraseFromParent();
        return translatedBranches[sequenceID];
      }

      // End the current block. Add it to the currently open function.
      builder.CreateRetVoid();
      openFunction->setName(sequenceID);
    }

    cout << endl;
  }

  if (verifyFunction(*openFunction, &errs())) {
    exit(1);
  }

  translatedBranches[sequenceID] = openFunction;
  return openFunction;
}

void Translator::translateToExecutable(string filename, bool onlyIR) {
  registerPietGlobals();
  Function *firstBranch =
      translateBranch(graph->getInitialNode(), graph->getCurrentDirection());

  auto mainFunction = cast<Function>(module.getOrInsertFunction(
      "main", IntegerType::getInt32Ty(context),
      IntegerType::getInt32Ty(context), IntegerType::getInt8Ty(context)));
  {
    Function::arg_iterator args = mainFunction->arg_begin();
    args[0].setName("argc");
    args[1].setName("argv");

    BasicBlock *entryBlock =
        BasicBlock::Create(context, "main_seq", mainFunction);
    builder.SetInsertPoint(entryBlock);
    builder.CreateCall(firstBranch);
    builder.CreateRet(ConstantInt::get(context, APInt(32, 0)));

    if (verifyFunction(*mainFunction, &errs())) {
      // TODO: throw parse exception to indicate Mondriaan bug.
      exit(1);
    }
  }

  if (verifyModule(module, &errs())) {
    // TODO: throw parse exception to indicate Mondriaan bug.
    exit(1);
  }

  // Optimise, please?

  if (onlyIR) {
    std::error_code writeError;
    auto outputStream = raw_fd_ostream(filename, writeError, sys::fs::F_None);
    module.print(outputStream, nullptr);
  } else {
    translateIRToExecutable(filename + ".o");
  }
}
} // namespace Piet
