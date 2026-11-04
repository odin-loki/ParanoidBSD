module;

export module pbsd.userland.talk.io;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/talk/io.c
export namespace pbsd::userland::usr_bin::talk::io {

[[nodiscard]] inline bool io_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::talk::io
