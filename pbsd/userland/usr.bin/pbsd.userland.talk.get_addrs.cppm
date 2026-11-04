module;

export module pbsd.userland.talk.get_addrs;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/talk/get_addrs.c
export namespace pbsd::userland::usr_bin::talk::get_addrs {

[[nodiscard]] inline bool get_addrs_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::talk::get_addrs
