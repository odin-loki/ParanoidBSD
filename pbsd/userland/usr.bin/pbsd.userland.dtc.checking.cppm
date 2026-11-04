module;

export module pbsd.userland.dtc.checking;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/dtc/checking.c
export namespace pbsd::userland::usr_bin::dtc::checking {

[[nodiscard]] inline bool checking_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::dtc::checking
