module;

export module pbsd.userland.dtc.string;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/dtc/string.c
export namespace pbsd::userland::usr_bin::dtc::string {

[[nodiscard]] inline bool string_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::dtc::string
