add_llvm_component_library(LLVMSM83AsmParser
  SM83AsmParser.cpp

  LINK_COMPONENTS
  MC
  MCParser
  SM83Desc
  SM83Info
  Support

  ADD_TO_COMPONENT
  SM83
)
