module;

export module pbsd.kernel.gtaskqueue;

export import pbsd.core;

/// Freestanding port of `sys/gtaskqueue.h` / `kern/subr_gtaskqueue.c`.
export namespace pbsd::kernel::gtaskqueue {

inline constexpr unsigned kNameLen = 32;

struct Softc {
    void* taskqueue{nullptr};
    void* uniq{nullptr};
    int cpu{-1};
    char name[kNameLen]{};
};

[[nodiscard]] inline Status validate_name_len(unsigned len) noexcept {
    if (len == 0 || len >= kNameLen) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status init(Softc& gt) noexcept {
    gt.taskqueue = nullptr;
    gt.uniq = nullptr;
    gt.cpu = -1;
    return Status::Ok;
}

} // namespace pbsd::kernel::gtaskqueue
