module;

export module pbsd.userland.talk.get_names;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/talk/get_names.c
export namespace pbsd::userland::usr_bin::talk::get_names {

[[nodiscard]] inline bool get_names_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::talk::get_names
