add_llvm_component_library(LLVMSM83Desc
  SM83MCAsmInfo.cpp
  SM83MCTargetDesc.cpp
  SM83InstPrinter.cpp

  LINK_COMPONENTS
  MC
  SM83Info
  Support

  ADD_TO_COMPONENT
  SM83
)
