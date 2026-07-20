module;
#include <cstdint>

export module pbsd.net.raw6;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet6/raw_ip6.h — raw IPv6 socket statistics.
export namespace pbsd::net::raw6 {

struct Stats {
    unsigned long long ipackets{};
    unsigned long long isum{};
    unsigned long long badsum{};
    unsigned long long nosock{};
    unsigned long long nosockmcast{};
    unsigned long long fullsock{};
    unsigned long long opackets{};
};

[[nodiscard]] inline Status account_input(Stats& st, unsigned len, bool bad_checksum) noexcept {
    ++st.ipackets;
    ++st.isum;
    if (bad_checksum) {
        ++st.badsum;
        return Status::Protocol;
    }
    (void)len;
    return Status::Ok;
}

[[nodiscard]] inline Status account_output(Stats& st, unsigned len) noexcept {
    ++st.opackets;
    (void)len;
    return Status::Ok;
}

[[nodiscard]] inline void record_nosock(Stats& st, bool multicast) noexcept {
    ++st.nosock;
    if (multicast) {
        ++st.nosockmcast;
    }
}

[[nodiscard]] inline void record_fullsock(Stats& st) noexcept {
    ++st.fullsock;
}

[[nodiscard]] inline unsigned long long total_drops(Stats const& st) noexcept {
    return st.badsum + st.nosock + st.fullsock;
}

} // namespace pbsd::net::raw6
