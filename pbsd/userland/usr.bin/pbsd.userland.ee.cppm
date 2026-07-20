module;

export module pbsd.userland.ee;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/ee/ee.c
export namespace pbsd::userland::usr_bin::ee {

[[nodiscard]] inline bool ee_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::ee
