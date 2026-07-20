module;

#include <cstdint>
#include <cstring>

export module pbsd.userland.libc.uuid;

/// uuid_create/uuid_create_nil/uuid_hash from hbsd/src/lib/libc/uuid/*.c
export namespace pbsd::userland::libc {

struct Uuid {
    std::uint32_t time_low{0};
    std::uint16_t time_mid{0};
    std::uint16_t time_hi_and_version{0};
    std::uint8_t clock_seq_hi_and_reserved{0};
    std::uint8_t clock_seq_low{0};
    std::uint8_t node[6]{};
};

inline void uuid_create_nil(Uuid& u) noexcept { u = Uuid{}; }

[[nodiscard]] inline bool uuid_is_nil(const Uuid& u) noexcept {
    Uuid nil{};
    return std::memcmp(&u, &nil, sizeof(u)) == 0;
}

[[nodiscard]] inline std::uint32_t uuid_hash(const Uuid& u) noexcept {
    std::uint32_t h = u.time_low;
    h ^= static_cast<std::uint32_t>(u.time_mid) << 16;
    h ^= static_cast<std::uint32_t>(u.time_hi_and_version) << 8;
    h ^= static_cast<std::uint32_t>(u.clock_seq_hi_and_reserved) << 24;
    h ^= static_cast<std::uint32_t>(u.clock_seq_low);
    for (int i = 0; i < 6; ++i) {
        h = (h << 5) + h + u.node[i];
    }
    return h;
}

} // namespace pbsd::userland::libc
