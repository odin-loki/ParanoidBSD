module;

export module pbsd.userland.random.randomize_fd;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/random/randomize_fd.c
export namespace pbsd::userland::usr_bin::random::randomize_fd {

[[nodiscard]] inline bool randomize_fd_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::random::randomize_fd
