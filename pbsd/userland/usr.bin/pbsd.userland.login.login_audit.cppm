module;

export module pbsd.userland.login.login_audit;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/login/login_audit.c
export namespace pbsd::userland::usr_bin::login::login_audit {

[[nodiscard]] inline bool login_audit_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::login::login_audit
