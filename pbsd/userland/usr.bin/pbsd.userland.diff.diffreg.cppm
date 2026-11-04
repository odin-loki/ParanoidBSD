module;

export module pbsd.userland.diff.diffreg;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/diff/diffreg.c
export namespace pbsd::userland::usr_bin::diff::diffreg {

[[nodiscard]] inline bool diffreg_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::diff::diffreg
