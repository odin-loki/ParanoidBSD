module;

export module pbsd.userland.clang.llvm_dwp.llvm_dwp_driver;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/clang/llvm-dwp/llvm-dwp-driver.c
export namespace pbsd::userland::usr_bin::clang::llvm_dwp::llvm_dwp_driver {

[[nodiscard]] inline bool llvm_dwp_llvm_dwp_driver_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::clang::llvm_dwp::llvm_dwp_driver
