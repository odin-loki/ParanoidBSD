module;

export module pbsd.userland.indent.pr_comment;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/indent/pr_comment.c
export namespace pbsd::userland::usr_bin::indent::pr_comment {

[[nodiscard]] inline bool pr_comment_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::indent::pr_comment
