module;

export module pbsd.userland.iscsictl.periphs;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/iscsictl/periphs.c
export namespace pbsd::userland::usr_bin::iscsictl::periphs {

[[nodiscard]] inline bool periphs_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::iscsictl::periphs
