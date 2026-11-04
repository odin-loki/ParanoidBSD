module;
#include <cstddef>

export module pbsd.userland.libc.net.link_proto;

export import pbsd.core;

/// link-level protocol helpers scaffold from hbsd/src/lib/libc/net/link_proto.c
export namespace pbsd::userland::libc::net {

[[nodiscard]] inline StatusOnly link_nametoaddr(const char* name, void* addr, std::size_t* alen) noexcept {
    (void)name;
    (void)addr;
    if (alen != nullptr) {
        *alen = 0;
    }
    return status_err(Status::NotImplemented);
}

} // namespace pbsd::userland::libc::net
