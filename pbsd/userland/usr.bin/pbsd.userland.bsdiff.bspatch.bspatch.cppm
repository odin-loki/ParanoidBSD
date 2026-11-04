module;

export module pbsd.userland.bsdiff.bspatch.bspatch;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/bsdiff/bspatch/bspatch.c
export namespace pbsd::userland::usr_bin::bsdiff::bspatch::bspatch {

[[nodiscard]] inline bool bspatch_bspatch_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::bsdiff::bspatch::bspatch
