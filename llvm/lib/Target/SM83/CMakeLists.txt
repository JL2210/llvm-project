set(LLVM_TARGET_DEFINITIONS SM83.td)

# tablegen(LLVM SM83GenCallingConv.inc -gen-callingconv)
tablegen(LLVM SM83GenInstrInfo.inc -gen-instr-info)
tablegen(LLVM SM83GenRegisterInfo.inc -gen-register-info)
tablegen(LLVM SM83GenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM SM83GenAsmMatcher.inc -gen-asm-matcher)

tablegen(LLVM SM83GenSubtargetInfo.inc -gen-subtarget)

add_public_tablegen_target(SM83CommonTableGen)

add_llvm_target(SM83CodeGen
  SM83TargetMachine.cpp
  )

add_subdirectory(AsmParser)
add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
