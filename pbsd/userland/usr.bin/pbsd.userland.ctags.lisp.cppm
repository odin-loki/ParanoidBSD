module;

export module pbsd.userland.ctags.lisp;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/ctags/lisp.c
export namespace pbsd::userland::usr_bin::ctags::lisp {

[[nodiscard]] inline bool lisp_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::ctags::lisp
