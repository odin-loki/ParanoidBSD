module;

export module pbsd.kernel.subr_hash;

import pbsd.core;

/// Freestanding port of `subr_hash.c` — hash table sizing helpers.
export namespace pbsd::kernel::subr_hash {

enum class HashFlag : unsigned {
    WaitOk = 0x01,
    NoWait = 0x02,
};

[[nodiscard]] inline unsigned hash_size(int elements) noexcept {
    if (elements <= 0) {
        return 0;
    }
    unsigned hashsize = 1;
    while (static_cast<int>(hashsize) <= elements) {
        hashsize <<= 1;
    }
    return hashsize >> 1;
}

[[nodiscard]] inline unsigned hash_mask(unsigned hashsize) noexcept {
    return hashsize - 1;
}

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    const bool wait = (flags & static_cast<unsigned>(HashFlag::WaitOk)) != 0;
    const bool nowait = (flags & static_cast<unsigned>(HashFlag::NoWait)) != 0;
    if (wait == nowait) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::subr_hash
