add_llvm_component_group(SM83)

set(LLVM_TARGET_DEFINITIONS SM83.td)

tablegen(LLVM SM83GenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM SM83GenCallingConv.inc -gen-callingconv)
tablegen(LLVM SM83GenGlobalISel.inc -gen-global-isel -warn-on-skipped-patterns)
tablegen(LLVM SM83GenInstrInfo.inc -gen-instr-info)
tablegen(LLVM SM83GenMCPseudoLowering.inc -gen-pseudo-lowering)
tablegen(LLVM SM83GenRegisterBank.inc -gen-register-bank)
tablegen(LLVM SM83GenRegisterInfo.inc -gen-register-info)
tablegen(LLVM SM83GenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM SM83GenPreLegalizeGICombiner.inc -gen-global-isel-combiner
              -combiners="SM83PreLegalizerCombiner")
tablegen(LLVM SM83GenO0PreLegalizeGICombiner.inc -gen-global-isel-combiner
              -combiners="SM83O0PreLegalizerCombiner")

add_public_tablegen_target(SM83CommonTableGen)

add_llvm_target(SM83CodeGen
  GISel/SM83CallLowering.cpp
  GISel/SM83LegalizerInfo.cpp
  GISel/SM83O0PreLegalizerCombiner.cpp
  GISel/SM83PreLegalizerCombiner.cpp
  GISel/SM83RegisterBankInfo.cpp
  GISel/SM83InstructionSelector.cpp
  SM83AsmPrinter.cpp
  SM83FrameLowering.cpp
  SM83InstrInfo.cpp
  SM83ISelLowering.cpp
  SM83MCInstLower.cpp
  SM83RegisterInfo.cpp
  SM83Subtarget.cpp
  SM83TargetMachine.cpp

  LINK_COMPONENTS
  Analysis
  AsmPrinter
  CodeGen
  CodeGenTypes
  Core
  GlobalISel
  MC
  SM83Desc
  SM83Info
  SelectionDAG
  Support
  Target

  ADD_TO_COMPONENT
  SM83
)

add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
