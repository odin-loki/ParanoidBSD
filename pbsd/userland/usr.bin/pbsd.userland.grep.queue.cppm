module;

export module pbsd.userland.grep.queue;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/grep/queue.c
export namespace pbsd::userland::usr_bin::grep::queue {

[[nodiscard]] inline bool queue_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::grep::queue
