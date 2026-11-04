module;

export module pbsd.userland.ed.glbl;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/ed/glbl.c
export namespace pbsd::userland::bin::ed::glbl {

[[nodiscard]] inline bool glbl_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::ed::glbl
