module;

export module pbsd.userland.ctags.yacc;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/ctags/yacc.c
export namespace pbsd::userland::usr_bin::ctags::yacc {

[[nodiscard]] inline bool yacc_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::ctags::yacc
