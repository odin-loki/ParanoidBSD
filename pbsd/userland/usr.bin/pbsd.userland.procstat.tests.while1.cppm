module;

export module pbsd.userland.procstat.tests.while1;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/procstat/tests/while1.c
export namespace pbsd::userland::usr_bin::procstat::tests::while1 {

[[nodiscard]] inline bool tests_while1_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::procstat::tests::while1
