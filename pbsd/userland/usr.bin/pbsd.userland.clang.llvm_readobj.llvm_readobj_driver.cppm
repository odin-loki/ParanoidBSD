module;

export module pbsd.userland.clang.llvm_readobj.llvm_readobj_driver;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/clang/llvm-readobj/llvm-readobj-driver.c
export namespace pbsd::userland::usr_bin::clang::llvm_readobj::llvm_readobj_driver {

[[nodiscard]] inline bool llvm_readobj_llvm_readobj_driver_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::clang::llvm_readobj::llvm_readobj_driver
