module;

export module pbsd.userland.pr.egetopt;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/pr/egetopt.c
export namespace pbsd::userland::usr_bin::pr::egetopt {

[[nodiscard]] inline bool egetopt_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::pr::egetopt
