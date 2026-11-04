module;

export module pbsd.userland.ed.undo;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/ed/undo.c
export namespace pbsd::userland::bin::ed::undo {

[[nodiscard]] inline bool undo_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::ed::undo
