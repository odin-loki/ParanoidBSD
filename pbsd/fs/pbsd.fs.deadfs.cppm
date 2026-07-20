module;

export module pbsd.fs.deadfs;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/deadfs — reclaim / dead vnode ops scaffold.
export namespace pbsd::fs::deadfs {

enum class Op : unsigned char {
    Lookup = 0,
    Open = 1,
    Read = 2,
    Write = 3,
    Reclaim = 4,
};

[[nodiscard]] inline Status reject(Op /*op*/) noexcept {
    return Status::Invalid;
}

[[nodiscard]] inline bool is_dead_mount(bool marked_dead) noexcept {
    return marked_dead;
}

} // namespace pbsd::fs::deadfs
