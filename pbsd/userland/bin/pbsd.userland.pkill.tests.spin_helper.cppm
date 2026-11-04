module;

export module pbsd.userland.pkill.tests.spin_helper;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/pkill/tests/spin_helper.c
export namespace pbsd::userland::bin::pkill::tests::spin_helper {

[[nodiscard]] inline bool tests_spin_helper_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::pkill::tests::spin_helper
