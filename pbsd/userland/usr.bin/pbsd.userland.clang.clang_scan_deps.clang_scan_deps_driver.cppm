module;

export module pbsd.userland.clang.clang_scan_deps.clang_scan_deps_driver;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/clang/clang-scan-deps/clang-scan-deps-driver.c
export namespace pbsd::userland::usr_bin::clang::clang_scan_deps::clang_scan_deps_driver {

[[nodiscard]] inline bool clang_scan_deps_clang_scan_deps_driver_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::clang::clang_scan_deps::clang_scan_deps_driver
