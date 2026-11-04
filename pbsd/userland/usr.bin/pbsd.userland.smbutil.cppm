module;

export module pbsd.userland.smbutil;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/smbutil/smbutil.c
export namespace pbsd::userland::usr_bin::smbutil {

[[nodiscard]] inline bool smbutil_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::smbutil
