module;

export module pbsd.userland.talk.get_iface;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/talk/get_iface.c
export namespace pbsd::userland::usr_bin::talk::get_iface {

[[nodiscard]] inline bool get_iface_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::talk::get_iface
