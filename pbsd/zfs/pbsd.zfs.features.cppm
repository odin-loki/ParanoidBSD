module;
#include <cstdint>

export module pbsd.zfs.features;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/zfeature_common.h
export namespace pbsd::zfs::features {

enum class SpaFeature : int {
    None = -1,
    AsyncDestroy,
    EmptyBpobj,
    Lz4Compress,
    MultiVdevCrashDump,
    SpacemapHistogram,
    EnabledTxg,
    HoleBirth,
    ExtensibleDataset,
    EmbeddedData,
    Bookmarks,
    FsSsLimit,
    LargeBlocks,
    LargeDnode,
    Sha512,
    Skein,
    Edonr,
    UserobjAccounting,
    Encryption,
    ProjectQuota,
    DeviceRemoval,
    ObsoleteCounts,
    PoolCheckpoint,
    SpacemapV2,
    AllocationClasses,
    ResilverDefer,
    BookmarkV2,
    RedactionBookmarks,
    RedactedDatasets,
    BookmarkWritten,
    LogSpacemap,
    Livelist,
    DeviceRebuild,
    ZstdCompress,
    Draid,
    Zilsaxattr,
    HeadErrlog,
    Blake3,
    BlockCloning,
    AvzV2,
    RedactionListSpill,
    RaidzExpansion,
    FastDedup,
    Longname,
    LargeMicrozap,
    DynamicGangHeader,
    BlockCloningEndian,
    PhysicalRewrite,
    Count,
};

enum class FeatureFlag : unsigned int {
    ReadonlyCompat    = 1u << 0,
    Mos               = 1u << 1,
    ActivateOnEnable  = 1u << 2,
    PerDataset        = 1u << 3,
    NoUpgrade         = 1u << 4,
};

struct FeatureEntry {
    SpaFeature    feature{};
    const char*   guid{};
    FeatureFlag   flags{};
};

inline constexpr FeatureEntry kFeatureTable[] = {
    {SpaFeature::Lz4Compress, "com.delphix:lz4_compress", FeatureFlag::ReadonlyCompat},
    {SpaFeature::LargeBlocks, "org.illumos:large_blocks", FeatureFlag::ReadonlyCompat},
    {SpaFeature::LargeDnode, "org.zfsonlinux:large_dnode", FeatureFlag::ReadonlyCompat},
    {SpaFeature::Encryption, "com.datto:encryption", FeatureFlag::PerDataset},
    {SpaFeature::ZstdCompress, "org.illumos:zstd_compress", FeatureFlag::ReadonlyCompat},
    {SpaFeature::BlockCloning, "org.openzfs:block_cloning", FeatureFlag::PerDataset},
    {SpaFeature::FastDedup, "org.openzfs:fast_dedup", FeatureFlag::NoUpgrade},
    {SpaFeature::Longname, "org.openzfs:longname", FeatureFlag::ReadonlyCompat},
    {SpaFeature::DynamicGangHeader, "org.openzfs:dynamic_gang_header", static_cast<FeatureFlag>(0)},
    {SpaFeature::PhysicalRewrite, "org.openzfs:physical_rewrite", FeatureFlag::PerDataset},
    {SpaFeature::AsyncDestroy, "com.delphix:async_destroy", FeatureFlag::ReadonlyCompat},
    {SpaFeature::HoleBirth, "com.delphix:hole_birth", FeatureFlag::ReadonlyCompat},
    {SpaFeature::EmbeddedData, "com.delphix:embedded_data", FeatureFlag::ReadonlyCompat},
    {SpaFeature::Bookmarks, "com.delphix:bookmarks", FeatureFlag::ReadonlyCompat},
    {SpaFeature::ProjectQuota, "org.zfsonlinux:project_quota", FeatureFlag::ReadonlyCompat},
    {SpaFeature::DeviceRemoval, "com.delphix:device_removal", FeatureFlag::ReadonlyCompat},
    {SpaFeature::SpacemapV2, "org.openzfs:spacemap_v2", FeatureFlag::ReadonlyCompat},
    {SpaFeature::Draid, "org.openzfs:draid", FeatureFlag::ReadonlyCompat},
    {SpaFeature::Blake3, "org.openzfs:blake3", FeatureFlag::ReadonlyCompat},
    {SpaFeature::EmptyBpobj, "com.delphix:empty_bpobj", FeatureFlag::ReadonlyCompat},
    {SpaFeature::MultiVdevCrashDump, "org.illumos:crash_dump", FeatureFlag::ReadonlyCompat},
    {SpaFeature::SpacemapHistogram, "com.delphix:spacemap_histogram", FeatureFlag::ReadonlyCompat},
    {SpaFeature::EnabledTxg, "com.delphix:enabled_txg", FeatureFlag::Mos},
    {SpaFeature::ExtensibleDataset, "com.delphix:extensible_dataset", FeatureFlag::Mos},
    {SpaFeature::FsSsLimit, "com.joyent:filesystem_limits", FeatureFlag::ReadonlyCompat},
    {SpaFeature::Sha512, "org.illumos:sha512", FeatureFlag::ReadonlyCompat},
    {SpaFeature::Skein, "org.illumos:skein", FeatureFlag::ReadonlyCompat},
    {SpaFeature::Edonr, "org.illumos:edonr", FeatureFlag::ReadonlyCompat},
    {SpaFeature::UserobjAccounting, "com.delphix:userobj_accounting", FeatureFlag::ReadonlyCompat},
    {SpaFeature::ObsoleteCounts, "com.delphix:obsolete_counts", FeatureFlag::ReadonlyCompat},
    {SpaFeature::PoolCheckpoint, "com.delphix:pool_checkpoint", FeatureFlag::ReadonlyCompat},
    {SpaFeature::AllocationClasses, "org.openzfs:allocation_classes", FeatureFlag::ReadonlyCompat},
    {SpaFeature::ResilverDefer, "com.delphix:resilver_defer", FeatureFlag::ReadonlyCompat},
    {SpaFeature::BookmarkV2, "org.zfsonlinux:bookmark_v2", FeatureFlag::ReadonlyCompat},
    {SpaFeature::RedactionBookmarks, "com.delphix:redaction_bookmarks", FeatureFlag::ReadonlyCompat},
    {SpaFeature::RedactedDatasets, "com.delphix:redacted_datasets", FeatureFlag::PerDataset},
    {SpaFeature::BookmarkWritten, "com.delphix:bookmark_written", FeatureFlag::ReadonlyCompat},
    {SpaFeature::LogSpacemap, "com.delphix:log_spacemap", FeatureFlag::ReadonlyCompat},
    {SpaFeature::Livelist, "com.delphix:livelist", FeatureFlag::ReadonlyCompat},
    {SpaFeature::DeviceRebuild, "org.openzfs:device_rebuild", FeatureFlag::ReadonlyCompat},
    {SpaFeature::Zilsaxattr, "org.zfsonlinux:zilsaxattr", FeatureFlag::PerDataset},
    {SpaFeature::HeadErrlog, "com.delphix:head_errlog", FeatureFlag::ReadonlyCompat},
    {SpaFeature::AvzV2, "org.openzfs:avz_v2", FeatureFlag::ReadonlyCompat},
    {SpaFeature::RedactionListSpill, "com.delphix:redaction_list_spill", FeatureFlag::PerDataset},
    {SpaFeature::RaidzExpansion, "org.openzfs:raidz_expansion", FeatureFlag::ReadonlyCompat},
    {SpaFeature::LargeMicrozap, "org.openzfs:large_microzap", FeatureFlag::ReadonlyCompat},
    {SpaFeature::BlockCloningEndian, "org.openzfs:block_cloning_endian", FeatureFlag::ReadonlyCompat},
};

[[nodiscard]] inline constexpr std::size_t feature_table_size() noexcept {
    return sizeof(kFeatureTable) / sizeof(kFeatureTable[0]);
}

[[nodiscard]] inline constexpr bool feature_enabled(unsigned long long active,
                                                    SpaFeature f) noexcept {
    if (f == SpaFeature::None || f == SpaFeature::Count) {
        return false;
    }
    auto bit = static_cast<unsigned>(f);
    return (active & (1ull << bit)) != 0;
}

[[nodiscard]] inline Status validate_feature_set(unsigned long long active) noexcept {
    if (feature_enabled(active, SpaFeature::Encryption)
        && !feature_enabled(active, SpaFeature::EmbeddedData)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline constexpr const char* feature_guid(SpaFeature f) noexcept {
    for (auto const& e : kFeatureTable) {
        if (e.feature == f) {
            return e.guid;
        }
    }
    return nullptr;
}

} // namespace pbsd::zfs::features
