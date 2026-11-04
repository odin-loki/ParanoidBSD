module;

#include <cstddef>
#include <cstdint>

export module pbsd.userland.libthr.key;

import pbsd.core;

/// pthread key/TSS from hbsd/src/lib/libthr/thread/thr_spec.c
export namespace pbsd::userland::libthr {

inline constexpr std::size_t kMaxKeys = 128;

struct TssKey {
    std::uint32_t id{0};
    bool allocated{false};
    void (*destructor)(void*){nullptr};
};

inline std::uint32_t g_next_key_id{0};

[[nodiscard]] inline Status key_create(TssKey& key, void (*destructor)(void*)) noexcept {
    key.allocated = true;
    key.id = g_next_key_id++;
    key.destructor = destructor;
    return Status::Ok;
}

[[nodiscard]] inline Status key_delete(TssKey& key) noexcept {
    key = TssKey{};
    return Status::Ok;
}

[[nodiscard]] inline Status key_set(TssKey& key, void* value) noexcept {
    if (!key.allocated) {
        return Status::Invalid;
    }
    (void)value;
    return Status::Ok;
}

[[nodiscard]] inline void* key_get(const TssKey& key) noexcept {
    if (!key.allocated) {
        return nullptr;
    }
    return nullptr;
}

} // namespace pbsd::userland::libthr
