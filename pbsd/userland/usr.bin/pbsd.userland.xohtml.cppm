module;

export module pbsd.userland.xohtml;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/xohtml/xohtml.c
export namespace pbsd::userland::usr_bin::xohtml {

[[nodiscard]] inline bool xohtml_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::xohtml
