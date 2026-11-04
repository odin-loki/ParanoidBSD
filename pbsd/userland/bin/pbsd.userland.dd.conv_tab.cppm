module;

export module pbsd.userland.dd.conv_tab;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/dd/conv_tab.c
export namespace pbsd::userland::bin::dd::conv_tab {

[[nodiscard]] inline bool conv_tab_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::dd::conv_tab
