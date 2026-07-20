module;

export module pbsd.userland.mt;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mt/mt.c
export namespace pbsd::userland::usr_bin::mt {

[[nodiscard]] inline bool mt_status(char c) noexcept { return c == 's'; }

} // namespace pbsd::userland::usr_bin::mt
