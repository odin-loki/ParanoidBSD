module;

export module pbsd.userland.sockstat.tests.sockstat_test;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/sockstat/tests/sockstat_test.c
export namespace pbsd::userland::usr_bin::sockstat::tests::sockstat_test {

[[nodiscard]] inline bool tests_sockstat_test_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::sockstat::tests::sockstat_test
