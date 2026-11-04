module;

export module pbsd.userland.kdump.linux;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/kdump/linux.c
export namespace pbsd::userland::usr_bin::kdump::linux {

[[nodiscard]] inline bool linux_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::kdump::linux
