module;

export module pbsd.userland.bintrans.uuencode;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/bintrans/uuencode.c
export namespace pbsd::userland::usr_bin::bintrans::uuencode {

[[nodiscard]] inline bool uuencode_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::bintrans::uuencode
