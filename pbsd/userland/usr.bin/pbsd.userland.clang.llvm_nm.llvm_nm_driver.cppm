module;

export module pbsd.userland.clang.llvm_nm.llvm_nm_driver;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/clang/llvm-nm/llvm-nm-driver.c
export namespace pbsd::userland::usr_bin::clang::llvm_nm::llvm_nm_driver {

[[nodiscard]] inline bool llvm_nm_llvm_nm_driver_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::clang::llvm_nm::llvm_nm_driver
