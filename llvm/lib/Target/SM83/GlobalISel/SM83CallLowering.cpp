#include "SM83CallLowering.h"

#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"

#define GET_INSTRINFO_ENUM
#include "SM83GenInstrInfo.inc"

using namespace llvm;

bool SM83CallLowering::lowerReturn(
    MachineIRBuilder &MIRBuilder,
    const Value *Val, ArrayRef<Register> VRegs) const {
  if (Val)
    return false;
  MIRBuilder.buildInstr(SM83::RET);
  return true;
}

bool SM83CallLowering::lowerFormalArguments(
    MachineIRBuilder &MIRBuilder, const Function &F,
    ArrayRef<ArrayRef<Register>> VRegs) const {
  llvm_unreachable("Unimplemented!");
}

bool SM83CallLowering::lowerCall(
    MachineIRBuilder &MIRBuilder,
    CallLoweringInfo &Info) const {
  llvm_unreachable("Unimplemented!");
}
