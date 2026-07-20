module;
#include <cstdint>

export module pbsd.bifrost.msr_bitmap;

import pbsd.core;
import pbsd.bifrost.hypervisor;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/intel/vmx_msr.h — MSR bitmap access bits.
export namespace pbsd::bifrost::msr_bitmap {

inline constexpr unsigned kPageSize = 4096;
inline constexpr unsigned kLowRangeMax = 0x00001FFFu;
inline constexpr unsigned kHighRangeBase = 0xC0000000u;
inline constexpr unsigned kHighRangeMax = 0xC0001FFFu;
inline constexpr unsigned kReadBitmapOffset = 0;
inline constexpr unsigned kWriteBitmapOffset = 2048;

enum class Access : unsigned char {
    None = 0x0,
    Read = 0x1,
    Write = 0x2,
    ReadWrite = Read | Write,
};

[[nodiscard]] inline Status validate_msr(unsigned msr) noexcept {
    if (msr <= kLowRangeMax) {
        return Status::Ok;
    }
    if (msr >= kHighRangeBase && msr <= kHighRangeMax) {
        return Status::Ok;
    }
    return Status::Invalid;
}

[[nodiscard]] inline Status byte_index(unsigned msr, unsigned& out) noexcept {
    if (validate_msr(msr) != Status::Ok) {
        return Status::Invalid;
    }
    if (msr <= kLowRangeMax) {
        out = msr / 8;
    } else {
        out = 1024 + (msr - kHighRangeBase) / 8;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned bit_mask(unsigned msr) noexcept {
    return 1u << (msr & 0x7u);
}

struct Bitmap {
    unsigned char data[kPageSize]{};
};

[[nodiscard]] inline Status initialize(Bitmap& bm) noexcept {
    for (auto& b : bm.data) {
        b = 0xFF;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status set_access(Bitmap& bm, unsigned msr, Access access) noexcept {
    unsigned byte = 0;
    if (byte_index(msr, byte) != Status::Ok) {
        return Status::Invalid;
    }
    const auto mask = bit_mask(msr);
    if ((static_cast<unsigned>(access) & static_cast<unsigned>(Access::Read)) != 0) {
        bm.data[byte + kReadBitmapOffset] &= ~static_cast<unsigned char>(mask);
    } else {
        bm.data[byte + kReadBitmapOffset] |= static_cast<unsigned char>(mask);
    }
    if ((static_cast<unsigned>(access) & static_cast<unsigned>(Access::Write)) != 0) {
        bm.data[byte + kWriteBitmapOffset] &= ~static_cast<unsigned char>(mask);
    } else {
        bm.data[byte + kWriteBitmapOffset] |= static_cast<unsigned char>(mask);
    }
    return Status::Ok;
}

[[nodiscard]] inline hypervisor::Backend backend_for() noexcept {
    return hypervisor::Backend::Vmx;
}

} // namespace pbsd::bifrost::msr_bitmap
