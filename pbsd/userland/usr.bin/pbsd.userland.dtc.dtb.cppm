module;

export module pbsd.userland.dtc.dtb;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/dtc/dtb.c
export namespace pbsd::userland::usr_bin::dtc::dtb {

[[nodiscard]] inline bool dtb_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::dtc::dtb
