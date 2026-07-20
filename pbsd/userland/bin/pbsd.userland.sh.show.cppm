module;

export module pbsd.userland.sh.show;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/sh/show.c
export namespace pbsd::userland::bin::sh::show {

[[nodiscard]] inline bool show_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::sh::show
