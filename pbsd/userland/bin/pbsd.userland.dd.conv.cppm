module;

export module pbsd.userland.dd.conv;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/dd/conv.c
export namespace pbsd::userland::bin::dd::conv {

[[nodiscard]] inline bool conv_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::dd::conv
