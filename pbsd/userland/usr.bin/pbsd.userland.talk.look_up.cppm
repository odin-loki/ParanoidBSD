module;

export module pbsd.userland.talk.look_up;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/talk/look_up.c
export namespace pbsd::userland::usr_bin::talk::look_up {

[[nodiscard]] inline bool look_up_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::talk::look_up
