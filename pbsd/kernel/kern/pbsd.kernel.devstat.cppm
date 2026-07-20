module;

export module pbsd.kernel.devstat;

export import pbsd.core;

/// Wave 4/5 — devstat tags from sys/devstat.h.
export namespace pbsd::kernel::devstat {

inline constexpr unsigned kTypeDirect = 0;
inline constexpr unsigned kTypeSequential = 1;
inline constexpr unsigned kTypePass = 2;

enum class Priority : unsigned char {
    Other = 0,
    Batch = 1,
    Idle  = 2,
    Standby = 3,
    Interactive = 4,
};

[[nodiscard]] constexpr Status validate_type(unsigned t) noexcept {
    if (t > kTypePass) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::devstat
