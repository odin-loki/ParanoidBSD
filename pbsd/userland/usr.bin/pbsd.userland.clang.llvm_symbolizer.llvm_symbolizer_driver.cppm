module;

export module pbsd.userland.clang.llvm_symbolizer.llvm_symbolizer_driver;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/clang/llvm-symbolizer/llvm-symbolizer-driver.c
export namespace pbsd::userland::usr_bin::clang::llvm_symbolizer::llvm_symbolizer_driver {

[[nodiscard]] inline bool llvm_symbolizer_llvm_symbolizer_driver_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::clang::llvm_symbolizer::llvm_symbolizer_driver
