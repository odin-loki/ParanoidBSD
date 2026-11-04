module;
#include <cstdint>

export module pbsd.kernel.vnode;

export import pbsd.core;

/// Wave 4 — kernel-side vnode types/states/flags (sys/vnode.h).
export namespace pbsd::kernel::vnode {

enum class Vtype : unsigned char {
    VNon      = 0,
    VReg      = 1,
    VDir      = 2,
    VBlk      = 3,
    VChr      = 4,
    VLink     = 5,
    VSock     = 6,
    VFifo     = 7,
    VBad      = 8,
    VMarker   = 9,
    VLastType = VMarker,
};

enum class Vstate : unsigned char {
    Uninitialized = 0,
    Constructed   = 1,
    Destroying    = 2,
    Dead          = 3,
    VLastState    = Dead,
};

enum class VvFlag : unsigned short {
    Root         = 0x0001,
    Istty        = 0x0002,
    Nosync       = 0x0004,
    Eternaldev   = 0x0008,
    Cachedlabel  = 0x0010,
    Vmsizevnlock = 0x0020,
    Copyonwrite  = 0x0040,
    System       = 0x0080,
    Procdep      = 0x0100,
    Unlinked     = 0x0200,
    Deleted      = 0x0400,
    Md           = 0x0800,
    Forceinsmq   = 0x1000,
    Readlink     = 0x2000,
    Unref        = 0x4000,
    Crosslock    = 0x8000,
};

struct VtypeEntry {
    Vtype       type;
    const char* name;
};

inline constexpr VtypeEntry kVtypeTable[] = {
    {Vtype::VNon,   "VNON"},
    {Vtype::VReg,   "VREG"},
    {Vtype::VDir,   "VDIR"},
    {Vtype::VBlk,   "VBLK"},
    {Vtype::VChr,   "VCHR"},
    {Vtype::VLink,  "VLNK"},
    {Vtype::VSock,  "VSOCK"},
    {Vtype::VFifo,  "VFIFO"},
    {Vtype::VBad,   "VBAD"},
    {Vtype::VMarker, "VMARKER"},
};

struct VnodeStub {
    Vtype        v_type{Vtype::VNon};
    Vstate       v_state{Vstate::Uninitialized};
    unsigned short v_vflag{};
};

[[nodiscard]] inline unsigned vtype_table_size() noexcept {
    return static_cast<unsigned>(sizeof(kVtypeTable) / sizeof(kVtypeTable[0]));
}

[[nodiscard]] constexpr bool vtype_is_dev(Vtype t) noexcept {
    return t == Vtype::VChr || t == Vtype::VBlk;
}

[[nodiscard]] constexpr bool vtype_is_special(Vtype t) noexcept {
    return t == Vtype::VFifo || t == Vtype::VSock || vtype_is_dev(t);
}

[[nodiscard]] constexpr bool has_vv_flag(unsigned short flags, VvFlag f) noexcept {
    return (flags & static_cast<unsigned short>(f)) != 0;
}

[[nodiscard]] constexpr Status validate_vtype(Vtype t) noexcept {
    return static_cast<unsigned char>(t) <= static_cast<unsigned char>(Vtype::VLastType)
        ? Status::Ok
        : Status::Invalid;
}

[[nodiscard]] constexpr Status validate_vstate(Vstate s) noexcept {
    return static_cast<unsigned char>(s) <= static_cast<unsigned char>(Vstate::VLastState)
        ? Status::Ok
        : Status::Invalid;
}

[[nodiscard]] inline Result<const char*> vtype_name(Vtype t) noexcept {
    for (const auto& e : kVtypeTable) {
        if (e.type == t) {
            return result_ok(e.name);
        }
    }
    return result_err<const char*>(Status::NotFound);
}

[[nodiscard]] constexpr bool vnode_pollable(Vtype t) noexcept {
    return t == Vtype::VReg || t == Vtype::VFifo || t == Vtype::VSock
        || t == Vtype::VChr;
}

} // namespace pbsd::kernel::vnode
