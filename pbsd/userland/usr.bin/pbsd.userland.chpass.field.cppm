module;

export module pbsd.userland.chpass.field;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/chpass/field.c
export namespace pbsd::userland::usr_bin::chpass::field {

[[nodiscard]] inline bool field_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::chpass::field
