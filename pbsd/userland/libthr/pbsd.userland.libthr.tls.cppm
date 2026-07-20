module;

#include <array>
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libthr.tls;

import pbsd.core;
import pbsd.userland.libthr.key;
import pbsd.userland.libthr.thread;

/// thread-local storage slots from hbsd/src/lib/libthr/thread/thr_spec.c
export namespace pbsd::userland::libthr {

inline std::array<void*, kMaxKeys> g_tls_slots{};

[[nodiscard]] inline Status tls_set(TssKey& key, void* value) noexcept {
    if (!key.allocated || key.id >= kMaxKeys) {
        return Status::Invalid;
    }
    g_tls_slots[key.id] = value;
    return key_set(key, value);
}

[[nodiscard]] inline void* tls_get(const TssKey& key) noexcept {
    if (!key.allocated || key.id >= kMaxKeys) {
        return nullptr;
    }
    return g_tls_slots[key.id];
}

inline void tls_clear_thread() noexcept {
    for (auto& slot : g_tls_slots) {
        slot = nullptr;
    }
}

} // namespace pbsd::userland::libthr
