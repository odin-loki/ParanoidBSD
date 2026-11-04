module;

export module pbsd.userland.talk.msgs;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/talk/msgs.c
export namespace pbsd::userland::usr_bin::talk::msgs {

[[nodiscard]] inline bool msgs_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::talk::msgs
