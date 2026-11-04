module;

export module pbsd.userland.mandoc;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mandoc/mandoc.c
export namespace pbsd::userland::usr_bin::mandoc {

[[nodiscard]] inline bool mandoc_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::mandoc
