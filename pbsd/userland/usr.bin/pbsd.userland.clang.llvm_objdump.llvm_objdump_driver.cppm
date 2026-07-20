module;

export module pbsd.userland.clang.llvm_objdump.llvm_objdump_driver;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/clang/llvm-objdump/llvm-objdump-driver.c
export namespace pbsd::userland::usr_bin::clang::llvm_objdump::llvm_objdump_driver {

[[nodiscard]] inline bool llvm_objdump_llvm_objdump_driver_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::clang::llvm_objdump::llvm_objdump_driver
