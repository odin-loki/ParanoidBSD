module;

export module pbsd.userland.hexdump.conv;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/hexdump/conv.c
export namespace pbsd::userland::usr_bin::hexdump::conv {

[[nodiscard]] inline bool conv_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::hexdump::conv
