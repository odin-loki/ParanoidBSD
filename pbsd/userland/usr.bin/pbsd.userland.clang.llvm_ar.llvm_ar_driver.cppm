module;

export module pbsd.userland.clang.llvm_ar.llvm_ar_driver;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/clang/llvm-ar/llvm-ar-driver.c
export namespace pbsd::userland::usr_bin::clang::llvm_ar::llvm_ar_driver {

[[nodiscard]] inline bool llvm_ar_llvm_ar_driver_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::clang::llvm_ar::llvm_ar_driver
