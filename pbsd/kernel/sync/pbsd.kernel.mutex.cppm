module;
#include <cstdint>

export module pbsd.kernel.mutex;

import pbsd.core;

/// Freestanding port of `sys/mutex.h` lock types, options, and state bits.
export namespace pbsd::kernel::mutex {

inline constexpr unsigned kMtxDef       = 0x00000000u;
inline constexpr unsigned kMtxSpin      = 0x00000001u;
inline constexpr unsigned kMtxRecurse   = 0x00000004u;
inline constexpr unsigned kMtxNowitness = 0x00000008u;
inline constexpr unsigned kMtxNoprofile = 0x00000020u;
inline constexpr unsigned kMtxNew       = 0x00000040u;

inline constexpr unsigned kMtxQuiet = 0x00000001u;
inline constexpr unsigned kMtxDupok = 0x00000002u;

inline constexpr unsigned kMtxUnowned   = 0x00000000u;
inline constexpr unsigned kMtxRecursed  = 0x00000001u;
inline constexpr unsigned kMtxContested = 0x00000002u;
inline constexpr unsigned kMtxDestroyed = 0x00000004u;
inline constexpr unsigned kMtxFlagMask  = kMtxRecursed | kMtxContested | kMtxDestroyed;

struct MutexClass {
    unsigned init_flags{kMtxDef};
    unsigned lock_state{kMtxUnowned};
    unsigned owner_tid{0};
    unsigned recurse_count{0};
};

[[nodiscard]] constexpr bool is_spin(unsigned flags) noexcept {
    return (flags & kMtxSpin) != 0;
}

[[nodiscard]] constexpr bool allows_recurse(unsigned flags) noexcept {
    return (flags & kMtxRecurse) != 0;
}

[[nodiscard]] constexpr Status validate_init_flags(unsigned flags) noexcept {
    const unsigned known = kMtxDef | kMtxSpin | kMtxRecurse | kMtxNowitness | kMtxNoprofile
        | kMtxNew;
    if ((flags & ~known) != 0) {
        return Status::Invalid;
    }
    if ((flags & kMtxSpin) != 0 && (flags & kMtxRecurse) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status try_lock(MutexClass& m, unsigned tid) noexcept {
    if ((m.lock_state & kMtxDestroyed) != 0) {
        return Status::Invalid;
    }
    if (m.lock_state == kMtxUnowned) {
        m.lock_state = kMtxContested;
        m.owner_tid = tid;
        m.lock_state &= ~kMtxContested;
        return Status::Ok;
    }
    if (allows_recurse(m.init_flags) && m.owner_tid == tid) {
        ++m.recurse_count;
        m.lock_state |= kMtxRecursed;
        return Status::Ok;
    }
    m.lock_state |= kMtxContested;
    return Status::Denied;
}

[[nodiscard]] inline Status unlock(MutexClass& m, unsigned tid) noexcept {
    if ((m.lock_state & kMtxDestroyed) != 0) {
        return Status::Invalid;
    }
    if (m.owner_tid != tid) {
        return Status::Denied;
    }
    if ((m.lock_state & kMtxRecursed) != 0 && m.recurse_count > 0) {
        --m.recurse_count;
        if (m.recurse_count > 0) {
            return Status::Ok;
        }
        m.lock_state &= ~kMtxRecursed;
    }
    m.owner_tid = 0;
    m.lock_state = kMtxUnowned;
    return Status::Ok;
}

struct MtxTypeEntry {
    unsigned    flag;
    const char* name;
};

inline constexpr MtxTypeEntry kTypeTable[] = {
    {kMtxDef,   "DEF"},
    {kMtxSpin,  "SPIN"},
    {kMtxRecurse, "RECURSE"},
};

[[nodiscard]] inline unsigned type_table_size() noexcept {
    return static_cast<unsigned>(sizeof(kTypeTable) / sizeof(kTypeTable[0]));
}

} // namespace pbsd::kernel::mutex
