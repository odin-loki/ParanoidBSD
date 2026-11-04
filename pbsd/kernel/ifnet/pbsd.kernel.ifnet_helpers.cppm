module;
#include <cstdint>

export module pbsd.kernel.ifnet_helpers;

import pbsd.core;
import pbsd.kernel.ifnet;

/// PROVENANCE: hbsd/src/sys/net/if.c — ifnet flag/link helpers.
export namespace pbsd::kernel::ifnet_helpers {

[[nodiscard]] inline bool is_up(unsigned flags) noexcept {
    return (flags & static_cast<unsigned>(ifnet::Flag::Up)) != 0;
}

[[nodiscard]] inline bool is_running(unsigned flags) noexcept {
    return (flags & static_cast<unsigned>(ifnet::Flag::DrvRunning)) != 0;
}

[[nodiscard]] inline Status set_link(ifnet::IfnetStub& ifp,
                                   ifnet::LinkState ls) noexcept {
    ifp.if_link_state = ls;
    return Status::Ok;
}

[[nodiscard]] inline bool link_up(ifnet::IfnetStub const& ifp) noexcept {
    return ifp.if_link_state == ifnet::LinkState::Up;
}

} // namespace pbsd::kernel::ifnet_helpers
