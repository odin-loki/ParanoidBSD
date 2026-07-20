module;

export module pbsd.userland.ed.buf;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/ed/buf.c
export namespace pbsd::userland::bin::ed::buf {

[[nodiscard]] inline bool buf_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::ed::buf
