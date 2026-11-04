module;

export module pbsd.userland.date.vary;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/date/vary.c
export namespace pbsd::userland::bin::date::vary {

[[nodiscard]] inline bool vary_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::date::vary
