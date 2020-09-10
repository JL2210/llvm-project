set(LLVM_TARGET_DEFINITIONS SM83.td)

# tablegen(LLVM SM83GenCallingConv.inc -gen-callingconv)
tablegen(LLVM SM83GenInstrInfo.inc -gen-instr-info)
tablegen(LLVM SM83GenRegisterInfo.inc -gen-register-info)

add_public_tablegen_target(SM83CommonTableGen)

add_llvm_target(SM83CodeGen
  SM83TargetMachine.cpp
  )

add_subdirectory(TargetInfo)
