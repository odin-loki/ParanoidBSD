module;

export module pbsd.userland.clang.llvm_profdata.llvm_profdata_driver;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/clang/llvm-profdata/llvm-profdata-driver.c
export namespace pbsd::userland::usr_bin::clang::llvm_profdata::llvm_profdata_driver {

[[nodiscard]] inline bool llvm_profdata_llvm_profdata_driver_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::clang::llvm_profdata::llvm_profdata_driver
