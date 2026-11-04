module;

export module pbsd.userland.at.perm;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/at/perm.c
export namespace pbsd::userland::usr_bin::at::perm {

[[nodiscard]] inline bool perm_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::at::perm
