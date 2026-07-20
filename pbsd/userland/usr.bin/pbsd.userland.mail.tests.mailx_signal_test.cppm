module;

export module pbsd.userland.mail.tests.mailx_signal_test;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mail/tests/mailx_signal_test.c
export namespace pbsd::userland::usr_bin::mail::tests::mailx_signal_test {

[[nodiscard]] inline bool tests_mailx_signal_test_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mail::tests::mailx_signal_test
