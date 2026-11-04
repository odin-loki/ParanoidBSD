export module pbsd.kernel.cam;

export import pbsd.core;

/// Wave 4 — CAM kernel-side path/periph KPI (sys/cam/cam.h).
export namespace pbsd::kernel::cam {

inline constexpr unsigned kMaxCdbLen = 16;
inline constexpr unsigned kPathWildcard = ~0u;
inline constexpr unsigned kTargetWildcard = ~0u;

enum class RunQueue : unsigned {
    Host = 0,
    Bus = 1,
    Xpt = 2,
    Dev = 3,
    Normal = 4,
};

struct PathStub {
    unsigned path_id{};
    unsigned target_id{};
    unsigned lun_id{};
};

struct PeriphStub {
    PathStub path{};
    void* softc{};
    bool started{};
};

[[nodiscard]] constexpr Status validate_path(const PathStub& p) noexcept {
    if (p.path_id == kPathWildcard && p.target_id != kTargetWildcard) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr Status register_periph(PeriphStub& p) noexcept {
    if (p.started) {
        return Status::Busy;
    }
    if (validate_path(p.path) != Status::Ok) {
        return Status::Invalid;
    }
    p.started = true;
    return Status::Ok;
}

[[nodiscard]] constexpr Status schedule(RunQueue q) noexcept {
    if (static_cast<unsigned>(q) >= static_cast<unsigned>(RunQueue::Normal) + 1) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned runq_table_size() noexcept {
    return 5;
}

} // namespace pbsd::kernel::cam
