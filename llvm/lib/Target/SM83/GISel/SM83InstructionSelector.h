#ifndef LLVM_LIB_TARGET_SM83_GISEL_SM83INSTRUCTIONSELECTOR_H
#define LLVM_LIB_TARGET_SM83_GISEL_SM83INSTRUCTIONSELECTOR_H

namespace llvm {

class InstructionSelector;
class SM83TargetMachine;
class SM83Subtarget;
class SM83RegisterBankInfo;

InstructionSelector *createSM83InstructionSelector(const SM83TargetMachine &TM,
                                                   SM83Subtarget &Subtarget,
                                                   SM83RegisterBankInfo &RBI);

} // namespace llvm

#endif
