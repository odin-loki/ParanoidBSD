module;
#include <cstdint>

export module pbsd.net.tcp_lro_entry;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/tcp_lro.c — LRO entry bookkeeping.
export namespace pbsd::net::tcp_lro_entry {

struct Entry {
    std::uint32_t packets{};
    std::uint32_t bytes{};
    bool active{false};
};

[[nodiscard]] inline Status activate(Entry& e) noexcept {
    e.active = true;
    e.packets = 0;
    e.bytes = 0;
    return Status::Ok;
}

[[nodiscard]] inline Status append(Entry& e, std::uint32_t nbytes) noexcept {
    if (!e.active || nbytes == 0) {
        return Status::Invalid;
    }
    ++e.packets;
    e.bytes += nbytes;
    return Status::Ok;
}

} // namespace pbsd::net::tcp_lro_entry
