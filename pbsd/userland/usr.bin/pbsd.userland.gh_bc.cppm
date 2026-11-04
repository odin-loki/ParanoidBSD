module;

export module pbsd.userland.gh_bc;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/gh-bc/gh-bc.c
export namespace pbsd::userland::usr_bin::gh_bc {

[[nodiscard]] inline bool gh_bc_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::gh_bc
