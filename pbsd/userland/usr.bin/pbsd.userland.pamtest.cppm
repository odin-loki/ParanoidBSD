module;

export module pbsd.userland.pamtest;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/pamtest/pamtest.c
export namespace pbsd::userland::usr_bin::pamtest {

[[nodiscard]] inline bool pamtest_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::pamtest
