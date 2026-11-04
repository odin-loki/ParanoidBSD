export module pbsd.fs;

import pbsd.core;
export import pbsd.handles;
export import pbsd.fs.vop;
export import pbsd.fs.mount;
export import pbsd.fs.namei;
export import pbsd.fs.stat;
export import pbsd.fs.fcntl;
export import pbsd.fs.dirent;
export import pbsd.fs.attr;
export import pbsd.fs.lockf;
export import pbsd.fs.quota;
export import pbsd.fs.ufs;
export import pbsd.fs.tmpfs;
export import pbsd.fs.pipefs;
export import pbsd.fs.extattr;
export import pbsd.fs.nfs;
export import pbsd.fs.nullfs;
export import pbsd.fs.procfs;
export import pbsd.fs.devfs;
export import pbsd.fs.fdescfs;
export import pbsd.fs.linprocfs;
export import pbsd.fs.fusefs;
export import pbsd.fs.msdosfs;
export import pbsd.fs.cd9660;
export import pbsd.fs.unionfs;
export import pbsd.fs.isofs;
export import pbsd.fs.autofs;
export import pbsd.fs.ext2fs;
export import pbsd.fs.smbfs;
export import pbsd.fs.ffs;
export import pbsd.fs.p9fs;
export import pbsd.fs.udf;
export import pbsd.fs.specfs;
export import pbsd.fs.vfs_cache;
export import pbsd.fs.vfs_aio;
export import pbsd.fs.ufs_dir;
export import pbsd.fs.tmpfs_node;
export import pbsd.fs.null_vnode;
export import pbsd.fs.deadfs;
export import pbsd.fs.pseudofs;
export import pbsd.fs.fifofs;
export import pbsd.fs.ufs_inode;
export import pbsd.fs.vfs_bio;
export import pbsd.fs.vfs_mount;
export import pbsd.fs.mqueuefs;

export import pbsd.fs.vfs_subr;
export import pbsd.fs.vfs_lookup;
export import pbsd.fs.vfs_syscalls;
export import pbsd.fs.vfs_vnode;
export import pbsd.fs.fifo_vnops;
export import pbsd.fs.ufs_vnops;
export import pbsd.fs.ffs_vnops;
export import pbsd.fs.devfs_vnops;
export import pbsd.fs.procfs_vnops;
export import pbsd.fs.vfs_hash;
export import pbsd.fs.vfs_init;
export import pbsd.fs.vfs_conf;
export import pbsd.fs.ext2fs_inode;
export import pbsd.fs.nfs_commonport;
export import pbsd.fs.nfs_commonsubs;
export import pbsd.fs.union_subr;
export import pbsd.fs.union_vfsops;
export import pbsd.fs.union_vnops;
export import pbsd.fs.vfs_acl;
export import pbsd.fs.vfs_cluster;
export import pbsd.fs.vfs_default;
export import pbsd.fs.vfs_export;
export import pbsd.fs.vfs_extattr;
export import pbsd.fs.vfs_inotify;
export import pbsd.fs.vfs_mountroot;
export import pbsd.fs.vfs_vnops;
export import pbsd.fs.autofs_vnops;
export import pbsd.fs.cd9660_vnops;
export import pbsd.fs.dead_vnops;
export import pbsd.fs.ext2_subr;
export import pbsd.fs.ext2_vnops;
export import pbsd.fs.fdesc_vnops;
export import pbsd.fs.fuse_vnops;
export import pbsd.fs.msdosfs_vnops;
export import pbsd.fs.null_subr;
export import pbsd.fs.null_vnops;
export import pbsd.fs.p9fs_subr;
export import pbsd.fs.p9fs_vnops;
export import pbsd.fs.pseudofs_vnops;
export import pbsd.fs.smbfs_subr;
export import pbsd.fs.smbfs_vnops;
export import pbsd.fs.tmpfs_subr;
export import pbsd.fs.tmpfs_vnops;
export import pbsd.fs.udf_vnops;
export import pbsd.fs.autofs_vfsops;
export import pbsd.fs.cd9660_bmap;
export import pbsd.fs.cd9660_lookup;
export import pbsd.fs.cd9660_node;
export import pbsd.fs.cd9660_vfsops;
export import pbsd.fs.cuse;
export import pbsd.fs.devfs_devs;
export import pbsd.fs.devfs_dir;
export import pbsd.fs.devfs_rule;
export import pbsd.fs.devfs_vfsops;
export import pbsd.fs.ext2_acl;
export import pbsd.fs.ext2_alloc;
export import pbsd.fs.ext2_bmap;
export import pbsd.fs.ext2_lookup;
export import pbsd.fs.ext2_vfsops;
export import pbsd.fs.fuse_vfsops;
export import pbsd.fs.msdosfs_vfsops;
export import pbsd.fs.null_vfsops;
export import pbsd.fs.procfs_vfsops;
export import pbsd.fs.tmpfs_vfsops;
export import pbsd.fs.cd9660_iconv;
export import pbsd.fs.cd9660_rrip;
export import pbsd.fs.cd9660_util;
export import pbsd.fs.ext2_balloc;
export import pbsd.fs.ext2_csum;
export import pbsd.fs.ext2_extattr;
export import pbsd.fs.ext2_extents;
export import pbsd.fs.ext2_hash;
export import pbsd.fs.ext2_htree;
export import pbsd.fs.ext2_inode;
export import pbsd.fs.ext2_inode_cnv;
export import pbsd.fs.fdesc_vfsops;
export import pbsd.fs.fuse_device;
export import pbsd.fs.fuse_file;
export import pbsd.fs.fuse_internal;
export import pbsd.fs.fuse_io;
export import pbsd.fs.fuse_ipc;
export import pbsd.fs.fuse_main;
export import pbsd.fs.fuse_node;
export import pbsd.fs.mntfs_vnops;
export import pbsd.fs.msdosfs_conv;
export import pbsd.fs.msdosfs_denode;
export import pbsd.fs.msdosfs_fat;
export import pbsd.fs.msdosfs_iconv;
export import pbsd.fs.msdosfs_lookup;
export import pbsd.fs.nfs_clport;
export import pbsd.fs.nfs_clsubs;
export import pbsd.fs.nfs_clvnops;
export import pbsd.fs.nfs_clvfsops;
export import pbsd.fs.procfs_status;
export import pbsd.fs.udf_vfsops;
export import pbsd.fs.ufs_bmap;
export import pbsd.fs.ufs_quota;
export import pbsd.fs.unionfs_vnops;
/// Wave 6 — VFS / ZFS boundary façade.
/// PROVENANCE: hbsd/src/sys/kern/vnode_if.src, sys/kern/vfs_*.c
export namespace pbsd::fs {

enum class VnodeType : unsigned char {
    Regular = static_cast<unsigned char>(vop::Vtype::VRegular),
    Directory = static_cast<unsigned char>(vop::Vtype::VDir),
    Symlink = static_cast<unsigned char>(vop::Vtype::VLink),
    Device = static_cast<unsigned char>(vop::Vtype::VBlock),
};

struct VnodeObject {
    static void release(VnodeObject* p) noexcept { (void)p; }
};

using VnodeHandle = UniqueHandle<VnodeObject>;

struct OpenOptions {
    CapabilityRights want{CapabilityRights::Read};
    bool create{false};
    bool truncate{false};
};

class Vfs {
public:
    [[nodiscard]] Status mount_root() noexcept {
        mounted_ = true;
        return Status::Ok;
    }

    [[nodiscard]] bool mounted() const noexcept { return mounted_; }

    /// Capability-checked open (SI-2 / SI-1).
    [[nodiscard]] Result<VnodeHandle> open(const char* path, OpenOptions opts,
                                             LineageId lineage) noexcept {
        if (!mounted_ || path == nullptr) {
            return {Status::Invalid, VnodeHandle{}};
        }
        CapabilityRights rights = CapabilityRights::Read;
        if (opts.create || opts.truncate) {
            rights = rights | CapabilityRights::Write | CapabilityRights::Grant;
        }
        rights = narrow_rights(rights, opts.want);
        if (rights == CapabilityRights::None) {
            return {Status::Denied, VnodeHandle{}};
        }
        (void)path;
        return {Status::Ok, VnodeHandle{reinterpret_cast<VnodeObject*>(1), rights, lineage}};
    }

    [[nodiscard]] Status read(const VnodeHandle& vn, CapabilityRights need_read) noexcept {
        if (!vn.valid() || !vn.has_right(need_read)) {
            return Status::Denied;
        }
        return vop::check_vop(vn.rights(), vop::Op::Read);
    }

    [[nodiscard]] Status write(const VnodeHandle& vn, CapabilityRights need_write) noexcept {
        if (!vn.valid() || !vn.has_right(need_write)) {
            return Status::Denied;
        }
        return vop::check_vop(vn.rights(), vop::Op::Write);
    }

    [[nodiscard]] Status vop_dispatch(const VnodeHandle& vn, vop::Op op) noexcept {
        if (!vn.valid()) {
            return Status::Invalid;
        }
        return vop::check_vop(vn.rights(), op);
    }

private:
    bool mounted_{false};
};

} // namespace pbsd::fs
