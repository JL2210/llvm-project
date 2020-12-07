#ifndef LLVM_LIB_TARGET_SM83_GISEL_SM83INSTRUCTIONSELECTOR_H
#define LLVM_LIB_TARGET_SM83_GISEL_SM83INSTRUCTIONSELECTOR_H

#include "llvm/CodeGen/GlobalISel/InstructionSelector.h"

namespace llvm {

class MachineInstr;

class SM83InstructionSelector : public InstructionSelector {
public:
  SM83InstructionSelector() : InstructionSelector() {}

  bool select(MachineInstr &I) override;
  // tablegen-erated
  bool selectImpl(MachineInstr &I);
};

#endif
