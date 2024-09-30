set(LLVM_TARGET_DEFINITIONS SM83.td)

add_llvm_target(SM83CodeGen
  SM83TargetMachine.cpp
  )

add_subdirectory(TargetInfo)
add_subdirectory(AsmPrinter)
