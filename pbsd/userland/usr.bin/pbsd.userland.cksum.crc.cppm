module;

export module pbsd.userland.cksum.crc;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/cksum/crc.c
export namespace pbsd::userland::usr_bin::cksum::crc {

[[nodiscard]] inline bool crc_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::cksum::crc
