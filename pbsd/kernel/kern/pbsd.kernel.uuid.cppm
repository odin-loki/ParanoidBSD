module;
#include <cstdint>

export module pbsd.kernel.uuid;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/kern/kern_uuid.c — UUID v4 scaffold.
export namespace pbsd::kernel::uuid {

struct Uuid {
    std::uint32_t time_low{};
    std::uint16_t time_mid{};
    std::uint16_t time_hi_and_version{};
    std::uint8_t clock_seq_hi_and_reserved{};
    std::uint8_t clock_seq_low{};
    std::uint8_t node[6]{};
};

[[nodiscard]] inline Status validate(const Uuid& u) noexcept {
    const auto ver = static_cast<unsigned>((u.time_hi_and_version >> 12) & 0xF);
    if (ver != 4) {
        return Status::Invalid;
    }
    if ((u.clock_seq_hi_and_reserved & 0xC0u) != 0x80u) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status make_v4(Uuid& out, const std::uint8_t entropy[16]) noexcept {
    if (entropy == nullptr) {
        return Status::Invalid;
    }
    out.time_low = (static_cast<std::uint32_t>(entropy[0]) << 24)
                 | (static_cast<std::uint32_t>(entropy[1]) << 16)
                 | (static_cast<std::uint32_t>(entropy[2]) << 8)
                 | entropy[3];
    out.time_mid = static_cast<std::uint16_t>((entropy[4] << 8) | entropy[5]);
    out.time_hi_and_version =
        static_cast<std::uint16_t>(((entropy[6] << 8) | entropy[7]) & 0x0FFF) | 0x4000;
    out.clock_seq_hi_and_reserved = static_cast<std::uint8_t>((entropy[8] & 0x3F) | 0x80);
    out.clock_seq_low = entropy[9];
    for (unsigned i = 0; i < 6; ++i) {
        out.node[i] = entropy[10 + i];
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::uuid
