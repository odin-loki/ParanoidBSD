module;

export module pbsd.kernel.hints;

export import pbsd.core;

/// Freestanding port of `kern/subr_hints.c` — loader hints KPI.
export namespace pbsd::kernel::hints {

inline constexpr unsigned kMaxName = 64;
inline constexpr unsigned kMaxValue = 256;

struct Entry {
    const char* name{nullptr};
    const char* value{nullptr};
};

[[nodiscard]] inline Status validate_entry(const Entry& e) noexcept {
    if (e.name == nullptr || *e.name == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::hints
