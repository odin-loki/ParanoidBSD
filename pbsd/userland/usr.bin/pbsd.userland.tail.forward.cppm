module;

export module pbsd.userland.tail.forward;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/tail/forward.c
export namespace pbsd::userland::usr_bin::tail::forward {

[[nodiscard]] inline bool forward_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::tail::forward
