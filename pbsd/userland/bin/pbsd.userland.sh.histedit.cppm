module;

export module pbsd.userland.sh.histedit;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/sh/histedit.c
export namespace pbsd::userland::bin::sh::histedit {

[[nodiscard]] inline bool histedit_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::sh::histedit
