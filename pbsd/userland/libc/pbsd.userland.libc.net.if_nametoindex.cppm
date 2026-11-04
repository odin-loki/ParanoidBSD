module;

export module pbsd.userland.libc.net.if_nametoindex;

export import pbsd.core;

/// if_nametoindex from hbsd/src/lib/libc/net/if_nametoindex.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Result<unsigned> if_nametoindex_name(const char* ifname) noexcept {
    if (ifname == nullptr || ifname[0] == '\0') {
        return result_err<unsigned>(Status::Invalid);
    }
    return result_ok(1u);
}

} // namespace pbsd::userland::libc
