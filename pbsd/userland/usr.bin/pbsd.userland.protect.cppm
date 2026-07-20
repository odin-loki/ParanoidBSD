module;

export module pbsd.userland.protect;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/protect/protect.c
export namespace pbsd::userland::usr_bin::protect {

[[nodiscard]] inline bool protect_quiet(char c) noexcept { return c == 'q'; }

} // namespace pbsd::userland::usr_bin::protect
