module;
#include <cstdint>

export module pbsd.kernel.rtentry;

export import pbsd.core;
export import pbsd.kernel.route;

/// Wave 4 — rtentry kernel layout stubs (sys/net/route.h, net/route.c).
export namespace pbsd::kernel::rtentry {

using namespace pbsd::kernel::route;

inline constexpr unsigned kRtfUp       = static_cast<unsigned>(Flag::Up);
inline constexpr unsigned kRtfGateway  = static_cast<unsigned>(Flag::Gateway);
inline constexpr unsigned kRtfHost     = static_cast<unsigned>(Flag::Host);
inline constexpr unsigned kRtfReject   = static_cast<unsigned>(Flag::Reject);
inline constexpr unsigned kRtfBlackhole = static_cast<unsigned>(Flag::Blackhole);

struct RtentryStub {
    unsigned    rt_flags{};
    unsigned    rt_fibnum{};
    std::uint32_t rt_mtu{};
    std::uint16_t rt_weight{};
    bool        rt_gwroute{};
};

[[nodiscard]] constexpr bool is_host_route(unsigned flags) noexcept {
    return (flags & kRtfHost) != 0;
}

[[nodiscard]] constexpr bool is_gateway_route(unsigned flags) noexcept {
    return (flags & kRtfGateway) != 0;
}

[[nodiscard]] constexpr Status validate_rtentry(const RtentryStub& rt) noexcept {
    if (validate_flags(rt.rt_flags) != Status::Ok) {
        return Status::Invalid;
    }
    if (is_gateway_route(rt.rt_flags) && is_host_route(rt.rt_flags)) {
        return Status::Invalid;
    }
    if (rt.rt_mtu != 0 && rt.rt_mtu < 576) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr Status apply_gateway(RtentryStub& rt) noexcept {
    rt.rt_flags |= kRtfGateway;
    rt.rt_gwroute = true;
    return validate_rtentry(rt);
}

} // namespace pbsd::kernel::rtentry
