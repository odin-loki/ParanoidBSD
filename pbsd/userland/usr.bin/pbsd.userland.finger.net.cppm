module;

export module pbsd.userland.finger.net;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/finger/net.c
export namespace pbsd::userland::usr_bin::finger::net {

[[nodiscard]] inline bool net_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::finger::net
