module;

export module pbsd.userland.clang.clang.clang_driver;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/clang/clang/clang-driver.c
export namespace pbsd::userland::usr_bin::clang::clang::clang_driver {

[[nodiscard]] inline bool clang_clang_driver_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::clang::clang::clang_driver
