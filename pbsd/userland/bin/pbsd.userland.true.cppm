module;

export module pbsd.userland.true_cmd;

export import pbsd.core;

/// Port of BSD true(1) — always succeeds (exit 0).
export namespace pbsd::userland::bin::true_ {

[[nodiscard]] inline StatusOnly run(int /*argc*/, char* const* /*argv*/) noexcept {
    return status_ok();
}

[[nodiscard]] inline int exit_code() noexcept { return 0; }

} // namespace pbsd::userland::bin::true_
