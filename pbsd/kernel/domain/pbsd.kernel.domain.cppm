module;
#include <cstdint>

export module pbsd.kernel.domain;

export import pbsd.core;

/// Wave 4 — protocol domain flags (sys/sys/domain.h).
export namespace pbsd::kernel::domain {

inline constexpr int kAfUnspec  = 0;
inline constexpr int kAfLocal   = 1;
inline constexpr int kAfInet    = 2;
inline constexpr int kAfInet6   = 28;
inline constexpr int kAfLink    = 18;

inline constexpr unsigned kDomfUnloadable = 0x0004;

struct DomainStub {
    int       dom_family{};
    unsigned  dom_flags{};
    const char* dom_name{};
};

inline constexpr DomainStub kDomainTable[] = {
    {kAfLocal, 0, "local"},
    {kAfInet,  0, "inet"},
    {kAfInet6, 0, "inet6"},
    {kAfLink,  0, "link"},
};

[[nodiscard]] inline unsigned domain_table_size() noexcept {
    return static_cast<unsigned>(sizeof(kDomainTable) / sizeof(kDomainTable[0]));
}

[[nodiscard]] constexpr bool is_unloadable(unsigned flags) noexcept {
    return (flags & kDomfUnloadable) != 0;
}

[[nodiscard]] constexpr Status validate_family(int fam) noexcept {
    for (const auto& d : kDomainTable) {
        if (d.dom_family == fam) {
            return Status::Ok;
        }
    }
    return Status::Invalid;
}

[[nodiscard]] constexpr Status validate_domain(const DomainStub& dom) noexcept {
    if (dom.dom_name == nullptr) {
        return Status::Invalid;
    }
    return validate_family(dom.dom_family);
}

} // namespace pbsd::kernel::domain
