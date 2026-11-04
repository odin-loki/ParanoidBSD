module;

export module pbsd.userland.stty.gfmt;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/stty/gfmt.c
export namespace pbsd::userland::bin::stty::gfmt {

[[nodiscard]] inline bool gfmt_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::stty::gfmt
