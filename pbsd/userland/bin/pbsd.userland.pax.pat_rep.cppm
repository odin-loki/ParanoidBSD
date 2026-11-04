module;

export module pbsd.userland.pax.pat_rep;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/pax/pat_rep.c
export namespace pbsd::userland::bin::pax::pat_rep {

[[nodiscard]] inline bool pat_rep_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::pax::pat_rep
