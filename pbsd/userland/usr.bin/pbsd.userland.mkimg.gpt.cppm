module;

export module pbsd.userland.mkimg.gpt;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mkimg/gpt.c
export namespace pbsd::userland::usr_bin::mkimg::gpt {

[[nodiscard]] inline bool gpt_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mkimg::gpt
