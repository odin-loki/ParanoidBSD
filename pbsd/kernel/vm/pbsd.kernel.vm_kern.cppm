module;
#include <cstdint>

export module pbsd.kernel.vm_kern;

import pbsd.core;
import pbsd.kernel.vm;

/// PROVENANCE: hbsd/src/sys/vm/vm_kern.c — kernel map entry helpers.
export namespace pbsd::kernel::vm_kern {

inline constexpr std::uint64_t kKernMapMin = 0xFFFF'8000'0000'0000ull;

struct MapParams {
    std::uint64_t start{};
    std::uint64_t size{};
    unsigned char prot{};
};

[[nodiscard]] inline Status validate_kern_map(const MapParams& p) noexcept {
    if (p.size == 0) {
        return Status::Invalid;
    }
    if (p.start < kKernMapMin) {
        return Status::Invalid;
    }
    return vm::validate_wx_vm(p.prot);
}

[[nodiscard]] inline std::uint64_t map_end(const MapParams& p) noexcept {
    return p.start + p.size;
}

} // namespace pbsd::kernel::vm_kern
