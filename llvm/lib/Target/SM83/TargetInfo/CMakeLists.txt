include_directories(${CMAKE_CURRENT_BINARY_DIR}/..
                    ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_llvm_component_library(LLVMSM83Info
  SM83TargetInfo.cpp
)

add_dependencies(LLVMSM83Info SM83CommonTableGen)

