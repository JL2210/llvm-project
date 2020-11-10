#ifndef LLVM_LIB_TARGET_SM83_SM83ISELLOWERING_H
#define LLVM_LIB_TARGET_SM83_SM83ISELLOWERING_H

#include "SM83TargetMachine.h"

#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {

class SM83TargetMachine;

class SM83TargetLowering : public TargetLowering {
public:
  SM83TargetLowering(const SM83TargetMachine &TM)
    : TargetLowering(TM) {}

  bool supportSwiftError() const override { return false; }  
};

}

#endif
