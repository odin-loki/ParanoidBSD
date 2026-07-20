export module pbsd.zfs;

import pbsd.core;
export import pbsd.handles;
export import pbsd.zfs.ioctl;
export import pbsd.zfs.features;
export import pbsd.zfs.props;
export import pbsd.zfs.zio;
export import pbsd.zfs.spa;
export import pbsd.zfs.dmu;
export import pbsd.zfs.dnode;
export import pbsd.zfs.vdev;
export import pbsd.zfs.arc;
export import pbsd.zfs.dsl;
export import pbsd.zfs.zio_checksum;
export import pbsd.zfs.zap;
export import pbsd.zfs.metaslab;
export import pbsd.zfs.zil;
export import pbsd.zfs.l2arc;
export import pbsd.zfs.zio_flag;
export import pbsd.zfs.compress;
export import pbsd.zfs.dbuf;
export import pbsd.zfs.sa;
export import pbsd.zfs.zfetch;
export import pbsd.zfs.txg;
export import pbsd.zfs.uberblock;
export import pbsd.zfs.space_map;
export import pbsd.zfs.vdev_queue;
export import pbsd.zfs.spa_load;
export import pbsd.zfs.dsl_pool;
export import pbsd.zfs.dsl_dir;
export import pbsd.zfs.dsl_dataset;
export import pbsd.zfs.vdev_label;
export import pbsd.zfs.zil_header;
export import pbsd.zfs.vnops;
export import pbsd.zfs.vfsops;
export import pbsd.zfs.dsl_scan;
export import pbsd.zfs.arc_stats;
export import pbsd.zfs.zio_inject;
export import pbsd.zfs.fuid;
import pbsd.fs;

export import pbsd.zfs.blkptr;
export import pbsd.zfs.dmu_object;
export import pbsd.zfs.dmu_tx;
export import pbsd.zfs.dnode_sync;
export import pbsd.zfs.spa_config;
export import pbsd.zfs.vdev_mirror;
export import pbsd.zfs.bpobj;
export import pbsd.zfs.ddt_zap;
export import pbsd.zfs.dsl_deadlist;
export import pbsd.zfs.arc_evict;
export import pbsd.zfs.spa_misc;
export import pbsd.zfs.spa_errlog;
export import pbsd.zfs.abd;
export import pbsd.zfs.aggsum;
export import pbsd.zfs.bplist;
export import pbsd.zfs.bptree;
export import pbsd.zfs.ddt;
export import pbsd.zfs.ddt_log;
export import pbsd.zfs.ddt_stats;
export import pbsd.zfs.dmu_objset;
export import pbsd.zfs.dmu_recv;
export import pbsd.zfs.dmu_send;
export import pbsd.zfs.dmu_traverse;
export import pbsd.zfs.dmu_zfetch;
export import pbsd.zfs.dsl_bookmark;
export import pbsd.zfs.dsl_crypt;
export import pbsd.zfs.dsl_destroy;
export import pbsd.zfs.dsl_prop;
export import pbsd.zfs.dataset_kstats;
export import pbsd.zfs.dbuf_stats;
export import pbsd.zfs.blake3_zfs;
export import pbsd.zfs.bqueue;
export import pbsd.zfs.brt;
export import pbsd.zfs.btree;
export import pbsd.zfs.dmu_diff;
export import pbsd.zfs.dmu_direct;
export import pbsd.zfs.dmu_redact;
export import pbsd.zfs.dsl_deleg;
export import pbsd.zfs.dsl_synctask;
export import pbsd.zfs.dsl_userhold;
export import pbsd.zfs.edonr_zfs;
export import pbsd.zfs.fm;
export import pbsd.zfs.gzip;
export import pbsd.zfs.hkdf;
export import pbsd.zfs.lz4;
export import pbsd.zfs.lz4_zfs;
export import pbsd.zfs.lzjb;
export import pbsd.zfs.mmp;
export import pbsd.zfs.multilist;
export import pbsd.zfs.objlist;
export import pbsd.zfs.pathname;
export import pbsd.zfs.range_tree;
export import pbsd.zfs.sha256_zfs;
export import pbsd.zfs.refcount;
export import pbsd.zfs.spa_checkpoint;
export import pbsd.zfs.vdev_file;
export import pbsd.zfs.vdev_raidz;
export import pbsd.zfs.zap_leaf;
export import pbsd.zfs.zcp;
export import pbsd.zfs.zvol;
/// Wave 6 — OpenZFS port track (see docs/migration/ZFS_CXX23.md).
export namespace pbsd::zfs {

struct DatasetObject {
    static void release(DatasetObject* p) noexcept { (void)p; }
};

using DatasetHandle = UniqueHandle<DatasetObject>;

enum class DatasetKind : unsigned char {
    FileSystem = static_cast<unsigned char>(ioctl::ZfsType::Filesystem),
    Volume     = static_cast<unsigned char>(ioctl::ZfsType::Volume),
    Snapshot   = static_cast<unsigned char>(ioctl::ZfsType::Snapshot),
};

class ZfsPool {
public:
    [[nodiscard]] Status import_pool(const char* guid) noexcept {
        if (guid == nullptr) {
            return Status::Invalid;
        }
        imported_ = true;
        return Status::Ok;
    }

    [[nodiscard]] Result<DatasetHandle> mount(const char* name, DatasetKind kind,
                                              CapabilityRights want,
                                              LineageId lineage) noexcept {
        if (!imported_ || name == nullptr) {
            return {Status::Invalid, DatasetHandle{}};
        }
        CapabilityRights rights = CapabilityRights::Read | CapabilityRights::Map;
        if (kind == DatasetKind::Volume) {
            rights = rights | CapabilityRights::Write;
        }
        rights = narrow_rights(rights, want);
        if (rights == CapabilityRights::None) {
            return {Status::Denied, DatasetHandle{}};
        }
        (void)name;
        return {Status::Ok, DatasetHandle{reinterpret_cast<DatasetObject*>(1), rights, lineage}};
    }

    [[nodiscard]] Status ioctl(const DatasetHandle& ds, ioctl::Ioc cmd) noexcept {
        if (!ds.valid()) {
            return Status::Invalid;
        }
        return ioctl::check_ioc(ds.rights(), cmd);
    }

    [[nodiscard]] Status require_feature(features::SpaFeature f) noexcept {
        if (!imported_) {
            return Status::Invalid;
        }
        if (!features::feature_enabled(active_features_, f)) {
            return Status::Denied;
        }
        return Status::Ok;
    }

    [[nodiscard]] bool imported() const noexcept { return imported_; }

private:
    bool imported_{false};
    unsigned long long active_features_{1ull << static_cast<unsigned>(features::SpaFeature::Lz4Compress)};
};

} // namespace pbsd::zfs
