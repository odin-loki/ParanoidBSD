module;
#include <cstdint>

export module pbsd.kernel.mac_label;

import pbsd.core;

/// Freestanding port of `security/mac/mac_label.c` — MAC label lifecycle.
export namespace pbsd::kernel::mac_label {

inline constexpr unsigned kFlagInitialized = 0x01;
inline constexpr unsigned kLabelBytes      = 256;

struct Label {
    unsigned flags{};
    std::uint8_t slots[kLabelBytes]{};
};

[[nodiscard]] inline Status init(Label& label) noexcept {
    label.flags = kFlagInitialized;
    for (unsigned i = 0; i < kLabelBytes; ++i) {
        label.slots[i] = 0;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status destroy(Label& label) noexcept {
    if ((label.flags & kFlagInitialized) == 0) {
        return Status::Invalid;
    }
    label.flags = 0;
    return Status::Ok;
}

[[nodiscard]] inline bool is_initialized(const Label& label) noexcept {
    return (label.flags & kFlagInitialized) != 0;
}

[[nodiscard]] inline Status copy(const Label& src, Label& dst) noexcept {
    if (!is_initialized(src)) {
        return Status::Invalid;
    }
    dst = src;
    return Status::Ok;
}

} // namespace pbsd::kernel::mac_label
