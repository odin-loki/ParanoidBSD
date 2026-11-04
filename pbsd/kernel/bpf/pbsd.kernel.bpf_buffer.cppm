module;
#include <cstdint>

export module pbsd.kernel.bpf_buffer;

import pbsd.core;

/// Freestanding port of `net/bpf_buffer.c` — BPF capture buffer management.
export namespace pbsd::kernel::bpf_buffer {

inline constexpr unsigned kDefaultSize = 4096;
inline constexpr unsigned kMaxSize     = 1048576;
inline constexpr unsigned kHeaderSize  = 16;

struct Buffer {
    unsigned capacity{kDefaultSize};
    unsigned head{};
    unsigned tail{};
    unsigned drops{};
    bool     active{};
};

struct Header {
    std::uint32_t ts_sec{};
    std::uint32_t ts_usec{};
    std::uint32_t caplen{};
    std::uint32_t len{};
};

[[nodiscard]] inline Status init(Buffer& buf, unsigned capacity) noexcept {
    if (capacity < kDefaultSize || capacity > kMaxSize) {
        return Status::Invalid;
    }
    buf.capacity = capacity;
    buf.head = 0;
    buf.tail = 0;
    buf.drops = 0;
    buf.active = true;
    return Status::Ok;
}

[[nodiscard]] inline Status append(Buffer& buf, unsigned len) noexcept {
    if (!buf.active) {
        return Status::Invalid;
    }
    const unsigned need = kHeaderSize + len;
    if (buf.head + need > buf.capacity) {
        ++buf.drops;
        return Status::Denied;
    }
    buf.head += need;
    return Status::Ok;
}

[[nodiscard]] inline unsigned available(const Buffer& buf) noexcept {
    if (!buf.active || buf.capacity <= buf.head) {
        return 0;
    }
    return buf.capacity - buf.head;
}

inline void reset(Buffer& buf) noexcept {
    buf.head = 0;
    buf.tail = 0;
}

} // namespace pbsd::kernel::bpf_buffer
