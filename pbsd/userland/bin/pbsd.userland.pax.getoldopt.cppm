module;

export module pbsd.userland.pax.getoldopt;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/pax/getoldopt.c
export namespace pbsd::userland::bin::pax::getoldopt {

[[nodiscard]] inline bool getoldopt_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::pax::getoldopt
