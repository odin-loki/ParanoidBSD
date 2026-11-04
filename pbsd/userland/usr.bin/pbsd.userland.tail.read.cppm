module;

export module pbsd.userland.tail.read;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/tail/read.c
export namespace pbsd::userland::usr_bin::tail::read {

[[nodiscard]] inline bool read_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::tail::read
