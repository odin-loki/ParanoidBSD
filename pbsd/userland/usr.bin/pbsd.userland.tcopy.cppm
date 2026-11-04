module;

export module pbsd.userland.tcopy;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/tcopy/tcopy.c
export namespace pbsd::userland::usr_bin::tcopy {

[[nodiscard]] inline bool tcopy_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::tcopy
