module;

export module pbsd.userland.lessecho;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/contrib/less/lessecho.c
export namespace pbsd::userland::usr_bin::lessecho {

[[nodiscard]] inline bool lessecho_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::lessecho
