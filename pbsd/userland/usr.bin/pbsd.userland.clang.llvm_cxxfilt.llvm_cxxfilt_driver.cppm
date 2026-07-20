module;

export module pbsd.userland.clang.llvm_cxxfilt.llvm_cxxfilt_driver;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/clang/llvm-cxxfilt/llvm-cxxfilt-driver.c
export namespace pbsd::userland::usr_bin::clang::llvm_cxxfilt::llvm_cxxfilt_driver {

[[nodiscard]] inline bool llvm_cxxfilt_llvm_cxxfilt_driver_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::clang::llvm_cxxfilt::llvm_cxxfilt_driver
