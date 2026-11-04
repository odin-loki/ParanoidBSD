module;

export module pbsd.userland.sh.jobs;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/sh/jobs.c
export namespace pbsd::userland::bin::sh::jobs {

[[nodiscard]] inline bool jobs_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::sh::jobs
