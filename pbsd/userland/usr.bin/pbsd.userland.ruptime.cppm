module;

export module pbsd.userland.ruptime;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/ruptime/ruptime.c
export namespace pbsd::userland::usr_bin::ruptime {

[[nodiscard]] inline bool ruptime_alive_only(char flag) noexcept { return flag == 'a'; }

} // namespace pbsd::userland::usr_bin::ruptime
