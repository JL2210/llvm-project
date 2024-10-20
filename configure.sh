cmake -G Ninja -DBUILD_SHARED_LIBS=On -DLLVM_USE_SPLIT_DWARF=On		\
  -DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD=SM83 -DLLVM_TARGETS_TO_BUILD=	\
  -DCMAKE_BUILD_TYPE=Debug -DLLVM_ENABLE_ASSERTIONS=On			\
  -DLLVM_BUILD_TESTS=On	-DLLVM_USE_LINKER=mold -DLLVM_CCACHE_BUILD=On	\
  -DLLVM_DEFAULT_TARGET_TRIPLE=sm83 -DLLVM_ENABLE_PROJECTS=clang	\
  -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++			\
  ../llvm
