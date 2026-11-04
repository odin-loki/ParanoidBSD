module;

export module pbsd.userland.ctags.C;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/ctags/C.c
export namespace pbsd::userland::usr_bin::ctags::C {

[[nodiscard]] inline bool C_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::ctags::C
