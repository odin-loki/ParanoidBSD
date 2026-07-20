module;

export module pbsd.userland.sh.redir;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/sh/redir.c
export namespace pbsd::userland::bin::sh::redir {

[[nodiscard]] inline bool redir_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::sh::redir
