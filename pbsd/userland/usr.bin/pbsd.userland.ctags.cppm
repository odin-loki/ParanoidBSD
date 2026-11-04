module;

export module pbsd.userland.ctags;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/ctags/ctags.c
export namespace pbsd::userland::usr_bin::ctags {

[[nodiscard]] inline bool ctags_recursive(char flag) noexcept { return flag == 'R'; }

} // namespace pbsd::userland::usr_bin::ctags
