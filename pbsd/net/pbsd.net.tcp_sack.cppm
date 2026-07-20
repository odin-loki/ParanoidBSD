module;
#include <cstdint>

export module pbsd.net.tcp_sack;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/tcp.h — TCP SACK option layout.
export namespace pbsd::net::tcp_sack {

inline constexpr unsigned kOptKind = 5;
inline constexpr unsigned kOptLen = 10;
inline constexpr unsigned kMaxBlocks = 4;

struct Block {
    std::uint32_t left{};
    std::uint32_t right{};
};

[[nodiscard]] inline Status validate_block(const Block& b) noexcept {
    if (b.right <= b.left) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_count(unsigned count) noexcept {
    if (count == 0 || count > kMaxBlocks) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned option_len(unsigned block_count) noexcept {
    if (validate_count(block_count) != Status::Ok) {
        return 0;
    }
    return 2 + block_count * kOptLen;
}

} // namespace pbsd::net::tcp_sack
