module;

export module pbsd.userland.diff.diffreg_new;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/diff/diffreg_new.c
export namespace pbsd::userland::usr_bin::diff::diffreg_new {

[[nodiscard]] inline bool diffreg_new_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::diff::diffreg_new
