add_llvm_component_library(LLVMSM83Info
  SM83TargetInfo.cpp

  LINK_COMPONENTS
  MC
  Support

  ADD_TO_COMPONENT
  SM83
)

