module;

export module pbsd.userland.less;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/less/less.c
export namespace pbsd::userland::usr_bin::less {

[[nodiscard]] inline bool less_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::less
