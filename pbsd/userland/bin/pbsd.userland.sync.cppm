module;

export module pbsd.userland.sync;

export import pbsd.core;

/// Port of hbsd/src/bin/sync/sync.c — trivial sync wrapper.
export namespace pbsd::userland::bin::sync {

[[nodiscard]] inline StatusOnly run() noexcept {
    return status_ok();
}

[[nodiscard]] inline int exit_code() noexcept {
    return 0;
}

} // namespace pbsd::userland::bin::sync
