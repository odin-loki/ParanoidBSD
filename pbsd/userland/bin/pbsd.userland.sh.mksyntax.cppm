module;

export module pbsd.userland.sh.mksyntax;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/sh/mksyntax.c
export namespace pbsd::userland::bin::sh::mksyntax {

[[nodiscard]] inline bool mksyntax_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::sh::mksyntax
