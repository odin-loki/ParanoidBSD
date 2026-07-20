module;

export module pbsd.userland.diff.pr;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/diff/pr.c
export namespace pbsd::userland::usr_bin::diff::pr {

[[nodiscard]] inline bool pr_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::diff::pr
