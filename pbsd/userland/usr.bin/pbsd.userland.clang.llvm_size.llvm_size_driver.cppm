module;

export module pbsd.userland.clang.llvm_size.llvm_size_driver;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/clang/llvm-size/llvm-size-driver.c
export namespace pbsd::userland::usr_bin::clang::llvm_size::llvm_size_driver {

[[nodiscard]] inline bool llvm_size_llvm_size_driver_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::clang::llvm_size::llvm_size_driver
