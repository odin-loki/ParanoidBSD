module;

export module pbsd.userland.sdiotool;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/sdiotool/sdiotool.c
export namespace pbsd::userland::usr_bin::sdiotool {

[[nodiscard]] inline bool sdiotool_verbose(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::sdiotool
