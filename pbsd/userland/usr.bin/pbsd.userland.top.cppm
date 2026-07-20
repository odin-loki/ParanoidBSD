module;

export module pbsd.userland.top;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/top/top.c
export namespace pbsd::userland::usr_bin::top {

[[nodiscard]] inline bool top_batch(char c) noexcept { return c == 'b'; }

} // namespace pbsd::userland::usr_bin::top
