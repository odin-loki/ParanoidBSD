module;

export module pbsd.userland.bsdiff;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/bsdiff/bsdiff/bsdiff.c
export namespace pbsd::userland::usr_bin::bsdiff {

[[nodiscard]] inline bool bsdiff_force(char c) noexcept { return c == 'f'; }

} // namespace pbsd::userland::usr_bin::bsdiff
