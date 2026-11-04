module;
#include <cstdint>

export module pbsd.fs.procfs;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/procfs/procfs.h — procfs node types.
export namespace pbsd::fs::procfs {

enum class NodeType : unsigned char {
    Root = 0,
    Curproc = 1,
    File = 2,
    Mem = 3,
    Regs = 4,
    Fpregs = 5,
    Notepg = 6,
    Map = 7,
};

enum class FileId : unsigned char {
    Exe = 0,
    Map = 1,
    Mem = 2,
    Regs = 3,
    Fpregs = 4,
    Notepg = 5,
    Cmdline = 6,
    Stat = 7,
};

[[nodiscard]] inline Status validate_node(NodeType t) noexcept {
    switch (t) {
    case NodeType::Root:
    case NodeType::Curproc:
    case NodeType::File:
    case NodeType::Mem:
    case NodeType::Regs:
    case NodeType::Fpregs:
    case NodeType::Notepg:
    case NodeType::Map:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline Status validate_pid(unsigned pid) noexcept {
    if (pid == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_file_id(FileId id) noexcept {
    switch (id) {
    case FileId::Exe:
    case FileId::Map:
    case FileId::Mem:
    case FileId::Regs:
    case FileId::Fpregs:
    case FileId::Notepg:
    case FileId::Cmdline:
    case FileId::Stat:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::fs::procfs
