module;

export module pbsd.userland.last;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/last/last.c
export namespace pbsd::userland::usr_bin::last {

[[nodiscard]] inline bool last_want_reboot(char flag) noexcept { return flag == 'x'; }

} // namespace pbsd::userland::usr_bin::last
