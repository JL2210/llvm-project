#ifndef LLVM_LIB_TARGET_SM83_SM83ISELLOWERING_H
#define LLVM_LIB_TARGET_SM83_SM83ISELLOWERING_H

#include "SM83.h"
#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {

class SM83TargetMachine;
class SM83Subtarget;

class SM83TargetLowering : public TargetLowering {
public:
  explicit SM83TargetLowering(const SM83TargetMachine &TM,
                              const SM83Subtarget &STI);

  bool supportSwiftError() const override { return false; }  
};

}

#endif
