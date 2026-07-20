module;

export module pbsd.userland.xargs.strnsubst;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/xargs/strnsubst.c
export namespace pbsd::userland::usr_bin::xargs::strnsubst {

[[nodiscard]] inline bool strnsubst_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::xargs::strnsubst
