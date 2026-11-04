module;

export module pbsd.userland.ps.keyword;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/ps/keyword.c
export namespace pbsd::userland::bin::ps::keyword {

[[nodiscard]] inline bool keyword_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::ps::keyword
