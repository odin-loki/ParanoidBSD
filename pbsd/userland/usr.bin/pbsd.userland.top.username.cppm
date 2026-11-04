module;

export module pbsd.userland.top.username;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/top/username.c
export namespace pbsd::userland::usr_bin::top::username {

[[nodiscard]] inline bool username_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::top::username
