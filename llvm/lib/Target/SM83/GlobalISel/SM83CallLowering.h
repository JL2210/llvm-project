#ifndef LLVM_LIB_TARGET_SM83_GLOBALISEL_SM83CALLLOWERING_H
#define LLVM_LIB_TARGET_SM83_GLOBALISEL_SM83CALLLOWERING_H

#include "SM83ISelLowering.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/GlobalISel/CallLowering.h"
#include "llvm/CodeGen/ValueTypes.h"

namespace llvm {

class SM83CallLowering : public CallLowering {
public:
  SM83CallLowering(const SM83TargetLowering &TL) : CallLowering(&TL) {}

  bool supportSwiftError() const override { return false; }

  bool lowerReturn(MachineIRBuilder &MIRBuilder, const Value *Val,
                   ArrayRef<Register> VRegs) const override;

  bool lowerFormalArguments(MachineIRBuilder &MIRBuilder, const Function &F,
                            ArrayRef<ArrayRef<Register>> VRegs) const override;

  bool lowerCall(MachineIRBuilder &MIRBuilder,
                 CallLoweringInfo &Info) const override;
};

} // namespace llvm

#endif
