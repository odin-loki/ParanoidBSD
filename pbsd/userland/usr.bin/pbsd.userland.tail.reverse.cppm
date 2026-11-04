module;

export module pbsd.userland.tail.reverse;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/tail/reverse.c
export namespace pbsd::userland::usr_bin::tail::reverse {

[[nodiscard]] inline bool reverse_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::tail::reverse
