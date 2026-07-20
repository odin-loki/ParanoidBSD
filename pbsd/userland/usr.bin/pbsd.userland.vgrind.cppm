module;

export module pbsd.userland.vgrind;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/vgrind/regexp.c
export namespace pbsd::userland::usr_bin::vgrind {

[[nodiscard]] inline bool vgrind_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::vgrind
