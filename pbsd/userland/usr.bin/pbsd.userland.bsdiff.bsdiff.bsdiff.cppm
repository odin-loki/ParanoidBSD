module;

export module pbsd.userland.bsdiff.bsdiff.bsdiff;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/bsdiff/bsdiff/bsdiff.c
export namespace pbsd::userland::usr_bin::bsdiff::bsdiff::bsdiff {

[[nodiscard]] inline bool bsdiff_bsdiff_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::bsdiff::bsdiff::bsdiff
