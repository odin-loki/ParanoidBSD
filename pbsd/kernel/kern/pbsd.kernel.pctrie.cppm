module;
#include <cstdint>

export module pbsd.kernel.pctrie;

export import pbsd.core;

/// Freestanding port of `sys/pctrie.h` / `kern/subr_pctrie.c`.
export namespace pbsd::kernel::pctrie {

struct Iter {
    void*  tree{nullptr};
    void*  node{nullptr};
    std::uint64_t index{};
    std::uint64_t limit{};
};

inline void reset(Iter& it) noexcept { it.node = nullptr; }

[[nodiscard]] inline bool is_reset(const Iter& it) noexcept { return it.node == nullptr; }

inline void init(Iter& it, void* tree) noexcept {
    it.tree = tree;
    it.node = nullptr;
    it.limit = 0;
}

[[nodiscard]] inline Status validate_key(std::uint64_t key) noexcept {
    (void)key;
    return Status::Ok;
}

} // namespace pbsd::kernel::pctrie
