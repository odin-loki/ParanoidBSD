module;
#include <cstdint>

export module pbsd.bifrost.tlb;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/vmm — guest TLB flush / INVVPID / INVEPT kinds.
export namespace pbsd::bifrost::tlb {

enum class InvKind : unsigned char {
    IndividualAddress = 0,
    SingleContext = 1,
    AllContexts = 2,
    AllContextsRetainingGlobals = 3,
};

enum class EptInvKind : unsigned char {
    SingleContext = 1,
    AllContexts = 2,
};

struct FlushRequest {
    InvKind kind{InvKind::SingleContext};
    std::uint64_t vpid{0};
    std::uint64_t gva{0};
};

[[nodiscard]] inline Status validate_vpid(std::uint64_t vpid) noexcept {
    return vpid != 0 && vpid <= 0xFFFFu ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline Status validate_flush(const FlushRequest& req) noexcept {
    if (req.kind == InvKind::IndividualAddress && req.gva == 0) {
        return Status::Invalid;
    }
    if (req.kind == InvKind::SingleContext) {
        return validate_vpid(req.vpid);
    }
    return Status::Ok;
}

} // namespace pbsd::bifrost::tlb
