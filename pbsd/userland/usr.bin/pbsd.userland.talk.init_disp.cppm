module;

export module pbsd.userland.talk.init_disp;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/talk/init_disp.c
export namespace pbsd::userland::usr_bin::talk::init_disp {

[[nodiscard]] inline bool init_disp_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::talk::init_disp
