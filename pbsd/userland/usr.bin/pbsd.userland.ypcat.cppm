module;

export module pbsd.userland.ypcat;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/ypcat/ypcat.c
export namespace pbsd::userland::usr_bin::ypcat {

[[nodiscard]] inline bool ypcat_map(char c) noexcept { return c == 'm'; }

} // namespace pbsd::userland::usr_bin::ypcat
