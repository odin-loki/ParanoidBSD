module;
#include <cstdint>

export module pbsd.kernel.rangeset;

export import pbsd.core;

/// Freestanding port of `sys/rangeset.h` / `kern/subr_rangeset.c`.
export namespace pbsd::kernel::rangeset {

struct Element {
    std::uint64_t start{};
    std::uint64_t end{};
};

struct Softc {
    void* trie{nullptr};
    void* dup_data{nullptr};
    void* free_data{nullptr};
    void* data_ctx{nullptr};
    unsigned alloc_flags{};
};

[[nodiscard]] inline Status validate_range(std::uint64_t start, std::uint64_t end) noexcept {
    if (end < start) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status init(Softc& rs) noexcept {
    rs.trie = nullptr;
    rs.dup_data = nullptr;
    rs.free_data = nullptr;
    rs.data_ctx = nullptr;
    rs.alloc_flags = 0;
    return Status::Ok;
}

} // namespace pbsd::kernel::rangeset
