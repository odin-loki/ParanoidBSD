module;

export module pbsd.userland.sh.cd;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/sh/cd.c
export namespace pbsd::userland::bin::sh::cd {

[[nodiscard]] inline bool cd_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::sh::cd
