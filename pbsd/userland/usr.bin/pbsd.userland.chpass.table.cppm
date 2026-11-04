module;

export module pbsd.userland.chpass.table;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/chpass/table.c
export namespace pbsd::userland::usr_bin::chpass::table {

[[nodiscard]] inline bool table_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::chpass::table
