module;

export module pbsd.userland.talk.invite;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/talk/invite.c
export namespace pbsd::userland::usr_bin::talk::invite {

[[nodiscard]] inline bool invite_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::talk::invite
