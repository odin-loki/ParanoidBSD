module;

export module pbsd.userland.lesskey;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/contrib/less/lesskey.c
export namespace pbsd::userland::usr_bin::lesskey {

[[nodiscard]] inline bool lesskey_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::lesskey
