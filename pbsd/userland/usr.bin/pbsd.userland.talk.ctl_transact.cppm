module;

export module pbsd.userland.talk.ctl_transact;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/talk/ctl_transact.c
export namespace pbsd::userland::usr_bin::talk::ctl_transact {

[[nodiscard]] inline bool ctl_transact_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::talk::ctl_transact
