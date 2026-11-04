module;

export module pbsd.userland.ofed;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/ofed/ofed.c
export namespace pbsd::userland::usr_bin::ofed {

[[nodiscard]] inline bool ofed_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::ofed
