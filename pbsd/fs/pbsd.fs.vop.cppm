module;
#include <cstddef>
#include <cstdint>

export module pbsd.fs.vop;

import pbsd.core;
import pbsd.rights;

using pbsd::CapabilityRights;
using pbsd::has_right;

/// PROVENANCE: hbsd/src/sys/kern/vnode_if.src — VFS op index table.
export namespace pbsd::fs::vop {

/// Vnode types (enum vtype from hbsd/src/sys/sys/vnode.h).
enum class Vtype : unsigned char {
    VNon     = 0,  // VNON
    VChar    = 1,  // VCHR
    VDir     = 2,  // VDIR
    VBlock   = 3,  // VBLK
    VRegular = 4,  // VREG
    VLink    = 5,  // VLNK
    VSocket  = 6,  // VSOCK
    VFifo    = 7,  // VFIFO
    VBad     = 8,  // VBAD
    VMarker  = 9,  // VMARKER
    VLastType = 9, // VLASTTYPE
};

enum class Vstate : unsigned char {
    Uninitialized = 0,
    Constructed   = 1,
    Destroying    = 2,
    Dead          = 3,
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

enum class Op : unsigned char {
    IsLocked = 0,
    Lookup,
    CachedLookup,
    Create,
    Whiteout,
    Mknod,
    Open,
    Close,
    FplookupVexec,
    FplookupSymlink,
    Access,
    Accessx,
    Stat,
    Getattr,
    Setattr,
    UpdateAtime,
    Read,
    ReadPgcache,
    Write,
    Ioctl,
    Poll,
    Kqfilter,
    Revoke,
    Fsync,
    Remove,
    Link,
    Rename,
    Mkdir,
    Rmdir,
    Symlink,
    Readdir,
    Readlink,
    Abortop,
    Activate,
    Deactivate,
    Inactive,
    Reclaim,
    Advlock,
    Advlockasync,
    Advlockpoll,
    Print,
    Pagein,
    Pageout,
    Getpages,
    Putpages,
    Getacl,
    Setacl,
    Closeextattr,
    Getextattr,
    Setextattr,
    Listextattr,
    Deleteextattr,
    Getwritemount,
    Addwritemount,
    Vptofh,
    Vptocnp,
    Aclcheck,
    Aclfrommode,
    Aclposix1,
    Getvattr,
    Setvattr,
    Pathconf,
    Advlockpat,
    Copyfilerange,
    OffloadActivate,
    OffloadDeactivate,
    OffloadRead,
    OffloadWrite,
    Seek,
    Seekhole,
    Seekdata,
    Copy,
    Getattrlite,
    Setattrlite,
    Needinval,
    Markatime,
    Markdirty,
    Markclean,
    Getlk,
    Setlk,
    Unsetlk,
    Setlkasync,
    Getlkasync,
    Unsetlkasync,
    Count = 83,
};

struct OpEntry {
    Op          op{};
    const char* name{};
    CapabilityRights need_read{CapabilityRights::None};
    CapabilityRights need_write{CapabilityRights::None};
};

inline constexpr OpEntry kVopTable[] = {
    {Op::IsLocked, "vop_islocked", CapabilityRights::Read, CapabilityRights::None},
    {Op::Lookup, "vop_lookup", CapabilityRights::Read, CapabilityRights::None},
    {Op::CachedLookup, "vop_cachedlookup", CapabilityRights::Read, CapabilityRights::None},
    {Op::Create, "vop_create", CapabilityRights::Read, CapabilityRights::Write | CapabilityRights::Grant},
    {Op::Open, "vop_open", CapabilityRights::Read, CapabilityRights::Write},
    {Op::Close, "vop_close", CapabilityRights::Read, CapabilityRights::None},
    {Op::Access, "vop_access", CapabilityRights::Read, CapabilityRights::None},
    {Op::Accessx, "vop_accessx", CapabilityRights::Read, CapabilityRights::None},
    {Op::Stat, "vop_stat", CapabilityRights::Read, CapabilityRights::None},
    {Op::Getattr, "vop_getattr", CapabilityRights::Read, CapabilityRights::None},
    {Op::Setattr, "vop_setattr", CapabilityRights::Read, CapabilityRights::Write},
    {Op::Read, "vop_read", CapabilityRights::Read, CapabilityRights::None},
    {Op::Write, "vop_write", CapabilityRights::Read, CapabilityRights::Write},
    {Op::Readdir, "vop_readdir", CapabilityRights::Read, CapabilityRights::None},
    {Op::Readlink, "vop_readlink", CapabilityRights::Read, CapabilityRights::None},
    {Op::Symlink, "vop_symlink", CapabilityRights::Read, CapabilityRights::Write | CapabilityRights::Grant},
    {Op::Link, "vop_link", CapabilityRights::Read, CapabilityRights::Write | CapabilityRights::Grant},
    {Op::Rename, "vop_rename", CapabilityRights::Read, CapabilityRights::Write | CapabilityRights::Destroy},
    {Op::Remove, "vop_remove", CapabilityRights::Read, CapabilityRights::Write | CapabilityRights::Destroy},
    {Op::Mkdir, "vop_mkdir", CapabilityRights::Read, CapabilityRights::Write | CapabilityRights::Grant},
    {Op::Rmdir, "vop_rmdir", CapabilityRights::Read, CapabilityRights::Write | CapabilityRights::Destroy},
    {Op::Ioctl, "vop_ioctl", CapabilityRights::Read, CapabilityRights::Write},
    {Op::Poll, "vop_poll", CapabilityRights::Read, CapabilityRights::None},
    {Op::Fsync, "vop_fsync", CapabilityRights::Read, CapabilityRights::Write},
    {Op::Revoke, "vop_revoke", CapabilityRights::Read, CapabilityRights::Destroy},
    {Op::Getacl, "vop_getacl", CapabilityRights::Read, CapabilityRights::None},
    {Op::Setacl, "vop_setacl", CapabilityRights::Read, CapabilityRights::Write},
    {Op::Advlock, "vop_advlock", CapabilityRights::Read, CapabilityRights::Write},
    {Op::Copyfilerange, "vop_copyfilerange", CapabilityRights::Read, CapabilityRights::Write},
    {Op::Seek, "vop_seek", CapabilityRights::Read, CapabilityRights::None},
    {Op::Seekhole, "vop_seekhole", CapabilityRights::Read, CapabilityRights::None},
    {Op::Seekdata, "vop_seekdata", CapabilityRights::Read, CapabilityRights::None},
};

[[nodiscard]] inline constexpr std::size_t vop_table_size() noexcept {
    return sizeof(kVopTable) / sizeof(kVopTable[0]);
}

[[nodiscard]] inline Status check_vop(CapabilityRights vn_rights, Op op) noexcept {
    for (auto const& e : kVopTable) {
        if (e.op != op) {
            continue;
        }
        if (!has_right(vn_rights, e.need_read)) {
            return Status::Denied;
        }
        if (e.need_write != CapabilityRights::None && !has_right(vn_rights, e.need_write)) {
            return Status::Denied;
        }
        return Status::Ok;
    }
    return Status::Invalid;
}

[[nodiscard]] inline constexpr bool vtype_is_dir(Vtype t) noexcept {
    return t == Vtype::VDir;
}

[[nodiscard]] inline constexpr bool vtype_is_dev(Vtype t) noexcept {
    return t == Vtype::VChar || t == Vtype::VBlock;
}

[[nodiscard]] inline constexpr bool vv_has(unsigned short flags, VvFlag f) noexcept {
    return (flags & static_cast<unsigned short>(f)) != 0;
}

[[nodiscard]] inline Status validate_vtype(Vtype t) noexcept {
    if (static_cast<unsigned char>(t) > static_cast<unsigned char>(Vtype::VLastType)) {
        return Status::Invalid;
    }
    if (t == Vtype::VBad || t == Vtype::VMarker) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_vstate(Vstate s) noexcept {
    if (static_cast<unsigned char>(s) > static_cast<unsigned char>(Vstate::Dead)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::fs::vop
