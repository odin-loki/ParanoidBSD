module;

export module pbsd.userland.false_cmd;

export import pbsd.core;

/// Port of BSD false(1) — always fails (exit 1).
export namespace pbsd::userland::bin::false_ {

[[nodiscard]] inline StatusOnly run(int /*argc*/, char* const* /*argv*/) noexcept {
    return status_err(Status::Invalid);
}

[[nodiscard]] inline int exit_code() noexcept { return 1; }

} // namespace pbsd::userland::bin::false_
