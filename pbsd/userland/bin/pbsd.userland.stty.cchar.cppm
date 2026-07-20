module;

export module pbsd.userland.stty.cchar;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/stty/cchar.c
export namespace pbsd::userland::bin::stty::cchar {

[[nodiscard]] inline bool cchar_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::stty::cchar
