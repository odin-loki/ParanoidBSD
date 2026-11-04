module;

export module pbsd.userland.cksum.crc32;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/cksum/crc32.c
export namespace pbsd::userland::usr_bin::cksum::crc32 {

[[nodiscard]] inline bool crc32_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::cksum::crc32
