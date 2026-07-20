module;

export module pbsd.userland.ed.re;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/ed/re.c
export namespace pbsd::userland::bin::ed::re {

[[nodiscard]] inline bool re_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::ed::re
