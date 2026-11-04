module;

export module pbsd.userland.clang.lld.lld_driver;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/clang/lld/lld-driver.c
export namespace pbsd::userland::usr_bin::clang::lld::lld_driver {

[[nodiscard]] inline bool lld_lld_driver_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::clang::lld::lld_driver
