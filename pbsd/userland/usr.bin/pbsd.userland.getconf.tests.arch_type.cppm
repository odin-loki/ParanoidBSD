module;

export module pbsd.userland.getconf.tests.arch_type;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/getconf/tests/arch_type.c
export namespace pbsd::userland::usr_bin::getconf::tests::arch_type {

[[nodiscard]] inline bool tests_arch_type_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::getconf::tests::arch_type
