module;

export module pbsd.userland.xzdec;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/contrib/xz/src/xzdec/xzdec.c
export namespace pbsd::userland::usr_bin::xzdec {

[[nodiscard]] inline bool xzdec_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::xzdec
