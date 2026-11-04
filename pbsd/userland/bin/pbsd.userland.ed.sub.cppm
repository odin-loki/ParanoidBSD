module;

export module pbsd.userland.ed.sub;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/ed/sub.c
export namespace pbsd::userland::bin::ed::sub {

[[nodiscard]] inline bool sub_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::ed::sub
