module;

export module pbsd.userland.libc.sys.fork;

export import pbsd.core;

/// fork from hbsd/src/lib/libc/sys/fork.c
export namespace pbsd::userland::libc {

enum class ForkRole : int { Parent = 0, Child = 1 };

[[nodiscard]] inline ForkRole fork_role(int pid) noexcept {
    return pid == 0 ? ForkRole::Child : ForkRole::Parent;
}

} // namespace pbsd::userland::libc
