module;
#include <cstdint>

export module pbsd.kernel.uma_dbg;

import pbsd.core;

/// Freestanding port of `vm/uma_dbg.c` UMA trash/redzone debug helpers.
export namespace pbsd::kernel::uma_dbg {

inline constexpr std::uint64_t kTrashPattern = 0xdeadc0dedeadc0deULL;

struct TrashCheck {
    std::uint64_t bad_words{};
};

[[nodiscard]] inline bool check_word(std::uint64_t word) noexcept {
    return word == kTrashPattern;
}

[[nodiscard]] inline Status trash_ctor(void* mem, unsigned size_bytes) noexcept {
    if (mem == nullptr || size_bytes == 0) {
        return Status::Invalid;
    }
    const unsigned count = size_bytes / sizeof(std::uint64_t);
    auto* p = static_cast<std::uint64_t*>(mem);
    for (unsigned i = 0; i < count; ++i) {
        if (!check_word(p[i])) {
            return Status::Invalid;
        }
    }
    return Status::Ok;
}

[[nodiscard]] inline Status trash_dtor(void* mem, unsigned size_bytes) noexcept {
    if (mem == nullptr || size_bytes == 0) {
        return Status::Invalid;
    }
    const unsigned count = size_bytes / sizeof(std::uint64_t);
    auto* p = static_cast<std::uint64_t*>(mem);
    for (unsigned i = 0; i < count; ++i) {
        p[i] = kTrashPattern;
    }
    return Status::Ok;
}

[[nodiscard]] inline TrashCheck scan(const std::uint64_t* words,
                                     unsigned count) noexcept {
    TrashCheck result{};
    for (unsigned i = 0; i < count; ++i) {
        if (!check_word(words[i])) {
            ++result.bad_words;
        }
    }
    return result;
}

} // namespace pbsd::kernel::uma_dbg
