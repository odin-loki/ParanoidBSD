module;
#include <cstdint>

export module pbsd.kernel.subr_uuid;

import pbsd.core;
import pbsd.kernel.uuid;

/// PROVENANCE: hbsd/src/sys/kern/kern_uuid.c — UUID parse/validate subr.
export namespace pbsd::kernel::subr_uuid {

[[nodiscard]] inline Status from_entropy(uuid::Uuid& out,
                                         const std::uint8_t entropy[16]) noexcept {
    return uuid::make_v4(out, entropy);
}

[[nodiscard]] inline Status check(const uuid::Uuid& u) noexcept {
    return uuid::validate(u);
}

[[nodiscard]] inline bool equal(const uuid::Uuid& a,
                                const uuid::Uuid& b) noexcept {
    return a.time_low == b.time_low
        && a.time_mid == b.time_mid
        && a.time_hi_and_version == b.time_hi_and_version
        && a.clock_seq_hi_and_reserved == b.clock_seq_hi_and_reserved
        && a.clock_seq_low == b.clock_seq_low
        && a.node[0] == b.node[0] && a.node[1] == b.node[1]
        && a.node[2] == b.node[2] && a.node[3] == b.node[3]
        && a.node[4] == b.node[4] && a.node[5] == b.node[5];
}

} // namespace pbsd::kernel::subr_uuid
