#ifndef LLVM_LIB_TARGET_SM83_SM83ISELLOWERING_H
#define LLVM_LIB_TARGET_SM83_SM83ISELLOWERING_H

#include "SM83.h"

#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/TargetLowering.h"
#include "llvm/IR/CallingConv.h"

namespace llvm {

class SM83TargetMachine;
class SM83Subtarget;

class SM83TargetLowering : public TargetLowering {
public:
  explicit SM83TargetLowering(const TargetMachine &TM,
                              const SM83Subtarget &STI);

  /// Selects the correct CCAssignFn for a given CallingConvention value.
  CCAssignFn *CCAssignFnForCall(CallingConv::ID CC, bool IsVarArg) const;
  CCAssignFn *CCAssignFnForReturn(CallingConv::ID CC) const;

  bool supportSwiftError() const override { return false; }
};

} // namespace llvm

#endif
