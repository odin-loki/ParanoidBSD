module;

export module pbsd.userland.pax.tables;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/pax/tables.c
export namespace pbsd::userland::bin::pax::tables {

[[nodiscard]] inline bool tables_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::pax::tables
