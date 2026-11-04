module;

export module pbsd.userland.bintrans.uudecode;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/bintrans/uudecode.c
export namespace pbsd::userland::usr_bin::bintrans::uudecode {

[[nodiscard]] inline bool uudecode_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::bintrans::uudecode
