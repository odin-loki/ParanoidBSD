module;
#include <cstdint>

export module pbsd.bifrost.ioport;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/vmm/vmm_ioport.c — guest port I/O dispatch.
export namespace pbsd::bifrost::ioport {

inline constexpr unsigned kPortCount = 65536;
inline constexpr unsigned kDefaultMmioBase = 0xE000'0000u;

enum class Access : unsigned char {
    In = 0,
    Out = 1,
    InString = 2,
    OutString = 3,
};

struct PortRequest {
    std::uint16_t port{0};
    unsigned size{1};
    Access access{Access::In};
};

[[nodiscard]] inline Status validate_port(std::uint16_t port) noexcept {
    (void)port;
    return Status::Ok;
}

[[nodiscard]] inline Status validate_request(const PortRequest& req) noexcept {
    if (req.size == 0 || req.size > 8) {
        return Status::Invalid;
    }
    return validate_port(req.port);
}

} // namespace pbsd::bifrost::ioport
