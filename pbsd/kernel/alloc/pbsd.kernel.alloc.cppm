module;
#include <cstddef>
#include <cstdint>
#include <new>

export module pbsd.kernel.alloc;

import pbsd.core;

/// Wave 4 — allocator zeroing (SI-4).
export namespace pbsd::kernel::alloc {

[[nodiscard]] inline void* zero_memory(void* dst, std::size_t size) noexcept {
    auto* p = static_cast<std::uint8_t*>(dst);
    for (std::size_t i = 0; i < size; ++i) {
        p[i] = 0;
    }
    return dst;
}

/// Freestanding zero-fill before returning to caller (SI-4).
[[nodiscard]] inline void* kmem_zalloc(std::size_t size) noexcept {
    // Hosted scaffold: single slab; kernel maps to uma_zalloc / malloc(9,M_ZERO).
    static constexpr std::size_t kSlabBytes = 65536;
    static std::uint8_t          slab[kSlabBytes]{};
    static std::size_t           offset{0};

    if (size == 0 || offset + size > kSlabBytes) {
        return nullptr;
    }
    void* ptr = slab + offset;
    offset += size;
    return zero_memory(ptr, size);
}

inline void kmem_free(void* ptr) noexcept {
    (void)ptr;
    // No-op in hosted scaffold; real kernel returns to uma.
}

template<typename T>
class ZeroedAlloc {
public:
    [[nodiscard]] static T* allocate() noexcept {
        void* raw = kmem_zalloc(sizeof(T));
        if (!raw) {
            return nullptr;
        }
        return new (raw) T{};
    }

    static void deallocate(T* p) noexcept {
        if (p) {
            p->~T();
            kmem_free(p);
        }
    }
};

} // namespace pbsd::kernel::alloc
