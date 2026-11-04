module;

export module pbsd.userland.ed.io;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/ed/io.c
export namespace pbsd::userland::bin::ed::io {

[[nodiscard]] inline bool io_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::ed::io
