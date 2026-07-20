module;

export module pbsd.userland.sdiff.edit;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/sdiff/edit.c
export namespace pbsd::userland::usr_bin::sdiff::edit {

[[nodiscard]] inline bool edit_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::sdiff::edit
