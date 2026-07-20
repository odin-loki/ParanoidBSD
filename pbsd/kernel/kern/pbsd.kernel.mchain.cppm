module;
#include <cstdint>

export module pbsd.kernel.mchain;

export import pbsd.core;

/// Freestanding port of `sys/mchain.h` — mbuf chain metadata helpers.
export namespace pbsd::kernel::mchain {

inline constexpr unsigned kMChainMaxLen = 65535u;

struct ChainMeta {
    unsigned length{};
    unsigned mtype{};
    bool     packet_header{};
};

[[nodiscard]] inline Status validate_length(unsigned len) noexcept {
    if (len == 0 || len > kMChainMaxLen) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status init(ChainMeta& cm, unsigned mtype) noexcept {
    cm.length = 0;
    cm.mtype = mtype;
    cm.packet_header = false;
    return Status::Ok;
}

[[nodiscard]] inline unsigned total_length(const ChainMeta& cm) noexcept {
    return cm.length;
}

} // namespace pbsd::kernel::mchain
