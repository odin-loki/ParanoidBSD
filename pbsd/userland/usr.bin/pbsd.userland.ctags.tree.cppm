module;

export module pbsd.userland.ctags.tree;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/ctags/tree.c
export namespace pbsd::userland::usr_bin::ctags::tree {

[[nodiscard]] inline bool tree_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::ctags::tree
