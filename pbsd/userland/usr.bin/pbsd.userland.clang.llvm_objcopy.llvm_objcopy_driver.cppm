module;

export module pbsd.userland.clang.llvm_objcopy.llvm_objcopy_driver;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/clang/llvm-objcopy/llvm-objcopy-driver.c
export namespace pbsd::userland::usr_bin::clang::llvm_objcopy::llvm_objcopy_driver {

[[nodiscard]] inline bool llvm_objcopy_llvm_objcopy_driver_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::clang::llvm_objcopy::llvm_objcopy_driver
