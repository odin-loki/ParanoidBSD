export module pbsd.rights;

import pbsd.core;

export namespace pbsd {

enum class CapabilityRights : unsigned int {
    None      = 0,
    Read      = 1u << 0,
    Write     = 1u << 1,
    Execute   = 1u << 2,
    Grant     = 1u << 3,
    Duplicate = 1u << 4,
    Destroy   = 1u << 5,
    Map       = 1u << 6,
    Irq       = 1u << 7,
    All       = 0xFFu,
};

using RightsMask = CapabilityRights;

[[nodiscard]] constexpr CapabilityRights operator|(CapabilityRights a, CapabilityRights b) noexcept {
    return static_cast<CapabilityRights>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}

[[nodiscard]] constexpr CapabilityRights operator&(CapabilityRights a, CapabilityRights b) noexcept {
    return static_cast<CapabilityRights>(static_cast<unsigned>(a) & static_cast<unsigned>(b));
}

[[nodiscard]] constexpr CapabilityRights operator^(CapabilityRights a, CapabilityRights b) noexcept {
    return static_cast<CapabilityRights>(static_cast<unsigned>(a) ^ static_cast<unsigned>(b));
}

[[nodiscard]] constexpr CapabilityRights operator~(CapabilityRights a) noexcept {
    return static_cast<CapabilityRights>(~static_cast<unsigned>(a) & static_cast<unsigned>(CapabilityRights::All));
}

constexpr CapabilityRights& operator|=(CapabilityRights& a, CapabilityRights b) noexcept {
    a = a | b;
    return a;
}

constexpr CapabilityRights& operator&=(CapabilityRights& a, CapabilityRights b) noexcept {
    a = a & b;
    return a;
}

[[nodiscard]] constexpr bool has_right(CapabilityRights set, CapabilityRights right) noexcept {
    return (set & right) != CapabilityRights::None;
}

[[nodiscard]] constexpr bool rights_empty(CapabilityRights set) noexcept {
    return set == CapabilityRights::None;
}

[[nodiscard]] constexpr bool rights_equal(CapabilityRights a, CapabilityRights b) noexcept {
    return a == b;
}

/// SI-1: child ⊆ parent
[[nodiscard]] constexpr bool rights_subset(CapabilityRights child, CapabilityRights parent) noexcept {
    return (static_cast<unsigned>(child) & ~static_cast<unsigned>(parent)) == 0u;
}

[[nodiscard]] consteval bool consteval_rights_subset(CapabilityRights child,
                                                     CapabilityRights parent) noexcept {
    return rights_subset(child, parent);
}

[[nodiscard]] constexpr CapabilityRights narrow_rights(CapabilityRights parent,
                                                       CapabilityRights want) noexcept {
    return parent & want;
}

[[nodiscard]] constexpr CapabilityRights widen_attempt(CapabilityRights parent,
                                                       CapabilityRights want) noexcept {
    return parent | want;
}

[[nodiscard]] constexpr CapabilityRights rights_union(CapabilityRights a,
                                                      CapabilityRights b) noexcept {
    return a | b;
}

[[nodiscard]] constexpr CapabilityRights rights_intersect(CapabilityRights a,
                                                        CapabilityRights b) noexcept {
    return a & b;
}

[[nodiscard]] constexpr CapabilityRights rights_diff(CapabilityRights a,
                                                   CapabilityRights b) noexcept {
    return a & ~b;
}

[[nodiscard]] constexpr Status check_grant(CapabilityRights parent, CapabilityRights child) noexcept {
    return rights_subset(child, parent) ? Status::Ok : Status::Denied;
}

[[nodiscard]] constexpr Status check_duplicate(CapabilityRights parent,
                                               CapabilityRights duplicate) noexcept {
    if (!has_right(parent, CapabilityRights::Duplicate)) {
        return Status::Denied;
    }
    return check_grant(parent, duplicate);
}

/// Compile-time SI-1 proof obligations for the rights algebra.
namespace proofs {

static_assert(consteval_rights_subset(CapabilityRights::Read, CapabilityRights::All));
static_assert(consteval_rights_subset(CapabilityRights::Read | CapabilityRights::Write,
                                      CapabilityRights::All));
static_assert(!consteval_rights_subset(CapabilityRights::All, CapabilityRights::Read));
static_assert(!consteval_rights_subset(CapabilityRights::Write, CapabilityRights::Read));
static_assert(rights_subset(narrow_rights(CapabilityRights::Read | CapabilityRights::Grant,
                                          CapabilityRights::Read),
                            CapabilityRights::Read | CapabilityRights::Grant));
static_assert(rights_equal(rights_union(CapabilityRights::Read, CapabilityRights::Write),
                           CapabilityRights::Read | CapabilityRights::Write));
static_assert(rights_empty(rights_diff(CapabilityRights::Read, CapabilityRights::Read)));

} // namespace proofs

} // namespace pbsd
