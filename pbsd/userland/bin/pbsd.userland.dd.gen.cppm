module;

export module pbsd.userland.dd.gen;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/dd/gen.c
export namespace pbsd::userland::bin::dd::gen {

[[nodiscard]] inline bool gen_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::dd::gen
