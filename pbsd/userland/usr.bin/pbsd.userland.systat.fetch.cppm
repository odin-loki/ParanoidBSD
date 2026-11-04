module;

export module pbsd.userland.systat.fetch;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/systat/fetch.c
export namespace pbsd::userland::usr_bin::systat::fetch {

[[nodiscard]] inline bool fetch_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::systat::fetch
