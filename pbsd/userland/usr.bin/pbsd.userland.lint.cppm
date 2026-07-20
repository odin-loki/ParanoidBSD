module;

export module pbsd.userland.lint;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/lint/lint.c
export namespace pbsd::userland::usr_bin::lint {

[[nodiscard]] inline bool lint_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::lint
