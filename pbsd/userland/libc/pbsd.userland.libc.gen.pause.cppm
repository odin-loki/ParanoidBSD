module;

export module pbsd.userland.libc.gen.pause;

export import pbsd.core;

/// pause from hbsd/src/lib/libc/gen/pause.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline bool signal_delivered(int pending_mask) noexcept {
    return pending_mask != 0;
}

} // namespace pbsd::userland::libc
