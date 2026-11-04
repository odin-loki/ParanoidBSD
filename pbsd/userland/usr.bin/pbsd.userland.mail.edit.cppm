module;

export module pbsd.userland.mail.edit;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mail/edit.c
export namespace pbsd::userland::usr_bin::mail::edit {

[[nodiscard]] inline bool edit_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mail::edit
