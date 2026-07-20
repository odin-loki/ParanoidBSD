module;

export module pbsd.userland.ctags.fortran;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/ctags/fortran.c
export namespace pbsd::userland::usr_bin::ctags::fortran {

[[nodiscard]] inline bool fortran_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::ctags::fortran
