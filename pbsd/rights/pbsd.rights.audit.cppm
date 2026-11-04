export module pbsd.rights.audit;

import pbsd.core;
import pbsd.rights;

/// Audit-specific capability rights (SI-1 subset checks).
export namespace pbsd::rights::audit {

enum class AuditRights : unsigned int {
    None = 0,
    Read = 1u << 0,
    Write = 1u << 1,
    Trail = 1u << 2,
    Trigger = 1u << 3,
    All = 0x0Fu,
};

[[nodiscard]] constexpr AuditRights operator|(AuditRights a, AuditRights b) noexcept {
    return static_cast<AuditRights>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}

[[nodiscard]] constexpr AuditRights operator&(AuditRights a, AuditRights b) noexcept {
    return static_cast<AuditRights>(static_cast<unsigned>(a) & static_cast<unsigned>(b));
}

[[nodiscard]] constexpr bool audit_subset(AuditRights child, AuditRights parent) noexcept {
    return (static_cast<unsigned>(child) & ~static_cast<unsigned>(parent)) == 0u;
}

[[nodiscard]] constexpr Status check_audit_grant(AuditRights parent, AuditRights child) noexcept {
    return audit_subset(child, parent) ? Status::Ok : Status::Denied;
}

} // namespace pbsd::rights::audit
