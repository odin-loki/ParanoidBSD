module;
#include <cstdint>

export module pbsd.geom.ctl;

import pbsd.core;
import pbsd.rights;

using pbsd::CapabilityRights;
using pbsd::has_right;
using pbsd::Status;

/// PROVENANCE: hbsd/src/sys/geom/geom_ctl.h, geom.h — GEOM ioctl/feature flags.
export namespace pbsd::geom::ctl {

inline constexpr unsigned kGctlVersion = 2;
inline constexpr unsigned kGctlArgMax  = 2048;

enum class ParamFlag : unsigned int {
    Rd           = 1,  // GCTL_PARAM_RD (VM_PROT_READ)
    Wr           = 2,  // GCTL_PARAM_WR
    Rw           = 3,  // GCTL_PARAM_RW
    Ascii        = 4,  // GCTL_PARAM_ASCII
    NameKernel   = 8,  // kernel-only
    ValueKernel  = 16,
    Changed      = 32,
};

enum class GeomFlag : unsigned int {
    Wither        = 0x01, // G_GEOM_WITHER
    VolatileBio   = 0x02, // G_GEOM_VOLATILE_BIO
    InAccess      = 0x04, // G_GEOM_IN_ACCESS
    AccessWait    = 0x08, // G_GEOM_ACCESS_WAIT
};

enum class TasteFlag : unsigned int {
    Normal      = 0, // G_TF_NORMAL
    Insist      = 1, // G_TF_INSIST
    Transparent = 2, // G_TF_TRANSPARENT
};

enum class ConsumerFlag : unsigned int {
    Spoiled       = 0x1,  // G_CF_SPOILED
    Orphan        = 0x4,  // G_CF_ORPHAN
    DirectSend    = 0x10, // G_CF_DIRECT_SEND
    DirectReceive = 0x20, // G_CF_DIRECT_RECEIVE
};

enum class ProviderFlag : unsigned int {
    Wither          = 0x2,  // G_PF_WITHER
    Orphan          = 0x4,  // G_PF_ORPHAN
    AcceptUnmapped  = 0x8,  // G_PF_ACCEPT_UNMAPPED
    DirectSend      = 0x10, // G_PF_DIRECT_SEND
    DirectReceive   = 0x20, // G_PF_DIRECT_RECEIVE
};

enum class GeomState : unsigned int {
    Failed  = 0, // G_STATE_FAILED
    Rebuild = 1, // G_STATE_REBUILD
    Resync  = 2, // G_STATE_RESYNC
    Active  = 3, // G_STATE_ACTIVE
};

inline constexpr unsigned kGeomVersion = 0x20041207; // G_VERSION_01

/// GEOM_CTL ioctl command base ('G' << 8 | GCTL_VERSION) — userland passes gctl_req.
inline constexpr unsigned kGeomCtlCmd = ('G' << 8) | kGctlVersion;

struct ReqArg {
    unsigned int nlen{0};
    const char*  name{};
    long long    offset{0};
    int          flag{0};
    int          len{0};
    const void*  value{};
};

struct ReqHeader {
    unsigned int version{kGctlVersion};
    unsigned int serial{0};
    unsigned int narg{0};
    ReqArg*      arg{nullptr};
    unsigned int lerror{0};
    const char*  error{};
};

[[nodiscard]] inline Status validate_req(const ReqHeader& req) noexcept {
    if (req.version != kGctlVersion) {
        return Status::Protocol;
    }
    if (req.narg > kGctlArgMax) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status check_param_rights(ParamFlag flags,
                                               CapabilityRights rights,
                                               bool writing) noexcept {
    if (writing) {
        if ((static_cast<unsigned>(flags) & static_cast<unsigned>(ParamFlag::Wr)) == 0) {
            return Status::Denied;
        }
        if (!has_right(rights, CapabilityRights::Write)) {
            return Status::Denied;
        }
    } else {
        if ((static_cast<unsigned>(flags) & static_cast<unsigned>(ParamFlag::Rd)) == 0) {
            return Status::Denied;
        }
        if (!has_right(rights, CapabilityRights::Read)) {
            return Status::Denied;
        }
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_geom_state(GeomState s) noexcept {
    return s <= GeomState::Active ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::geom::ctl
