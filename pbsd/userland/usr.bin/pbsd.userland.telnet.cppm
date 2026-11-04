module;

export module pbsd.userland.telnet;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/telnet/telnet.c
export namespace pbsd::userland::usr_bin::telnet {

[[nodiscard]] inline bool telnet_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::telnet
