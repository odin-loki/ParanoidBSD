module;

export module pbsd.userland.vi;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/vi/vi.c
export namespace pbsd::userland::usr_bin::vi {

[[nodiscard]] inline bool vi_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::vi
