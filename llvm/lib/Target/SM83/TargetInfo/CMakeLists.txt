add_llvm_component_library(LLVMSM83Info
  SM83TargetInfo.cpp

  LINK_COMPONENTS
  Support

  ADD_TO_COMPONENT
  SM83
)

