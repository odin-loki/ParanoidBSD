module;

export module pbsd.userland.dtc.fdt;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/dtc/fdt.c
export namespace pbsd::userland::usr_bin::dtc::fdt {

[[nodiscard]] inline bool fdt_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::dtc::fdt
