module;

export module pbsd.userland.talk.ctl;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/talk/ctl.c
export namespace pbsd::userland::usr_bin::talk::ctl {

[[nodiscard]] inline bool ctl_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::talk::ctl
