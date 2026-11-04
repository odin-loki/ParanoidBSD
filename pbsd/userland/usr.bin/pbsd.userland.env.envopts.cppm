module;

export module pbsd.userland.env.envopts;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/env/envopts.c
export namespace pbsd::userland::usr_bin::env::envopts {

[[nodiscard]] inline bool envopts_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::env::envopts
