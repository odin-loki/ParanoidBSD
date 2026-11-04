module;
#include <cstdint>

export module pbsd.net.tcp_offload;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/tcp_offload.c — TOE capability flags.
export namespace pbsd::net::tcp_offload {

enum class Cap : unsigned {
    Tso   = 0x0001,
    Lro   = 0x0002,
    Toe   = 0x0004,
    Checksum = 0x0008,
};

[[nodiscard]] inline Status validate_caps(unsigned caps) noexcept {
    constexpr unsigned kAll = static_cast<unsigned>(Cap::Tso)
                            | static_cast<unsigned>(Cap::Lro)
                            | static_cast<unsigned>(Cap::Toe)
                            | static_cast<unsigned>(Cap::Checksum);
    if ((caps & ~kAll) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool has_tso(unsigned caps) noexcept {
    return (caps & static_cast<unsigned>(Cap::Tso)) != 0;
}

} // namespace pbsd::net::tcp_offload
