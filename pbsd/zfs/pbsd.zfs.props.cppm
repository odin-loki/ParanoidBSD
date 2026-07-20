module;
#include <cstdint>

export module pbsd.zfs.props;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/zfs_prop.h — zprop types/attrs.
export namespace pbsd::zfs::props {

enum class PropType : unsigned char {
    Number = 0,
    String = 1,
    Index  = 2,
};

enum class PropAttr : unsigned char {
    Default         = 0,
    Readonly        = 1,
    Inherit         = 2,
    Onetime         = 3,
    OnetimeDefault  = 4,
};

enum class DatasetProp : unsigned int {
    Type        = 0,
    Creation    = 1,
    Used        = 2,
    Available   = 3,
    Referenced  = 4,
    Compress    = 5,
    Checksum    = 6,
    Recordsize  = 7,
    Mountpoint  = 8,
    Quota       = 9,
    Encryption  = 10,
};

struct PropEntry {
    DatasetProp prop{};
    PropType    type{};
    PropAttr    attr{};
    const char* name{};
};

inline constexpr PropEntry kPropTable[] = {
    {DatasetProp::Type, PropType::String, PropAttr::Readonly, "type"},
    {DatasetProp::Creation, PropType::Number, PropAttr::Readonly, "creation"},
    {DatasetProp::Used, PropType::Number, PropAttr::Readonly, "used"},
    {DatasetProp::Available, PropType::Number, PropAttr::Readonly, "available"},
    {DatasetProp::Compress, PropType::Index, PropAttr::Inherit, "compression"},
    {DatasetProp::Checksum, PropType::Index, PropAttr::Inherit, "checksum"},
    {DatasetProp::Recordsize, PropType::Number, PropAttr::Inherit, "recordsize"},
    {DatasetProp::Mountpoint, PropType::String, PropAttr::Inherit, "mountpoint"},
    {DatasetProp::Quota, PropType::Number, PropAttr::Default, "quota"},
    {DatasetProp::Encryption, PropType::Index, PropAttr::Onetime, "encryption"},
};

[[nodiscard]] inline constexpr std::size_t prop_table_size() noexcept {
    return sizeof(kPropTable) / sizeof(kPropTable[0]);
}

[[nodiscard]] inline Status validate_attr(PropAttr a) noexcept {
    if (static_cast<unsigned char>(a) > static_cast<unsigned char>(PropAttr::OnetimeDefault)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::zfs::props
