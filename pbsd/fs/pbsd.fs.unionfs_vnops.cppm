module;

export module pbsd.fs.unionfs_vnops;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/unionfs — unionfs vnode ops scaffold.
export namespace pbsd::fs::unionfs_vnops {

enum class Layer : unsigned char {
    Upper = 0,
    Lower = 1,
};

enum class Op : unsigned char {
    Lookup = 0,
    Create = 1,
    Whiteout = 2,
    Readdir = 3,
};

[[nodiscard]] inline Status validate(Op op, Layer layer) noexcept {
    switch (op) {
    case Op::Lookup:
    case Op::Readdir:
        return Status::Ok;
    case Op::Create:
    case Op::Whiteout:
        return layer == Layer::Upper ? Status::Ok : Status::Denied;
    }
    return Status::Invalid;
}

} // namespace pbsd::fs::unionfs_vnops
