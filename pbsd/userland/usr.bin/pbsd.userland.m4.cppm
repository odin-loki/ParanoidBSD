export module pbsd.userland.m4;

export import pbsd.userland.hosted;

/// Wave 2 — m4(1) option stubs from usr.bin/m4/.
export namespace pbsd::userland::m4 {

struct Options {
    bool prefix_builtins{};
    bool fatal_warnings{};
    bool synclines{};
    unsigned nesting_limit{1024};
};

[[nodiscard]] constexpr Status validate(const Options& o) noexcept {
    if (o.nesting_limit == 0 || o.nesting_limit > 1'000'000u) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::m4
