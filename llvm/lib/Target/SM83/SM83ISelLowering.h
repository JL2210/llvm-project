#ifndef LLVM_LIB_TARGET_SM83_SM83ISELLOWERING_H
#define LLVM_LIB_TARGET_SM83_SM83ISELLOWERING_H

#include "SM83TargetMachine.h"

#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {

class SM83TargetMachine;

class SM83TargetLowering : public TargetLowering {
  const SM83Subtarget &Subtarget;

public:
  explicit SM83TargetLowering(const SM83TargetMachine &TM,
                              const SM83Subtarget &STI);

  bool supportSwiftError() const override { return false; }  
};

}

#endif
