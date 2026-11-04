module;
#include <cstdint>

export module pbsd.kernel.if_mib;

import pbsd.core;

/// Freestanding port of `net/if_mib.h` — interface MIB sysctl data.
export namespace pbsd::kernel::if_mib {

inline constexpr unsigned kNameLen = 16;
inline constexpr int kMibSystem    = 1;
inline constexpr int kMibIfdata    = 2;

struct IfData {
    std::uint64_t ibytes{};
    std::uint64_t obytes{};
    std::uint64_t ipackets{};
    std::uint64_t opackets{};
    std::uint64_t ierrors{};
    std::uint64_t oerrors{};
    std::uint64_t collisions{};
    std::uint64_t imcasts{};
    std::uint64_t omcasts{};
};

struct IfMibData {
    char name[kNameLen]{};
    int  promisc_count{};
    int  flags{};
    int  snd_len{};
    int  snd_maxlen{};
    int  snd_drops{};
    IfData data{};
};

[[nodiscard]] inline Status validate_index(int ifindex) noexcept {
    if (ifindex <= 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status fill(IfMibData& md, int ifindex, char const* name) noexcept {
    if (validate_index(ifindex) != Status::Ok || name == nullptr) {
        return Status::Invalid;
    }
    for (unsigned i = 0; i < kNameLen - 1 && name[i] != '\0'; ++i) {
        md.name[i] = name[i];
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::if_mib
