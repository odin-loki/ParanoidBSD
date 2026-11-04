module;
#include <cstdint>

export module pbsd.kernel.protosw;

export import pbsd.core;

/// Wave 4 — protosw flags (sys/sys/protosw.h).
export namespace pbsd::kernel::protosw {

inline constexpr unsigned kPrAtomic       = 0x01;
inline constexpr unsigned kPrAddr         = 0x02;
inline constexpr unsigned kPrConnrequired = 0x04;
inline constexpr unsigned kPrWantrcvd     = 0x08;
inline constexpr unsigned kPrImplopcl     = 0x20;
inline constexpr unsigned kPrCapattach    = 0x80;
inline constexpr unsigned kPrSockbuf     = 0x100;

struct ProtoswStub {
    int       pr_type{};
    unsigned  pr_flags{};
    int       pr_protocol{};
};

[[nodiscard]] inline unsigned flag_table_size() noexcept {
    return 7;
}

[[nodiscard]] constexpr bool requires_connection(unsigned flags) noexcept {
    return (flags & kPrConnrequired) != 0;
}

[[nodiscard]] constexpr bool is_atomic(unsigned flags) noexcept {
    return (flags & kPrAtomic) != 0;
}

[[nodiscard]] constexpr Status validate_flags(unsigned flags) noexcept {
    if ((flags & kPrAddr) && !(flags & kPrAtomic)) {
        return Status::Invalid;
    }
    if ((flags & kPrConnrequired) && (flags & kPrAddr)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr Status validate_protosw(const ProtoswStub& pr) noexcept {
    if (pr.pr_protocol < 0) {
        return Status::Invalid;
    }
    return validate_flags(pr.pr_flags);
}

} // namespace pbsd::kernel::protosw
