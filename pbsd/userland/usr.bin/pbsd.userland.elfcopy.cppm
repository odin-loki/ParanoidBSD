module;

export module pbsd.userland.elfcopy;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/elfcopy/elfcopy.c
export namespace pbsd::userland::usr_bin::elfcopy {

[[nodiscard]] inline bool elfcopy_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::elfcopy
