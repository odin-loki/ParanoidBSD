module;

export module pbsd.userland.nc;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/nc/nc.c
export namespace pbsd::userland::usr_bin::nc {

[[nodiscard]] inline bool nc_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::nc
