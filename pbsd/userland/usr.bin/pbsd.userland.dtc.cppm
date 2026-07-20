module;

export module pbsd.userland.dtc;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/dtc/dtc.c
export namespace pbsd::userland::usr_bin::dtc {

[[nodiscard]] inline bool dtc_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::dtc
