module;

export module pbsd.userland.fortune.fortune.fortune;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/fortune/fortune/fortune.c
export namespace pbsd::userland::usr_bin::fortune::fortune::fortune {

[[nodiscard]] inline bool fortune_fortune_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::fortune::fortune::fortune
