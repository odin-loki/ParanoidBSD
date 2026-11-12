<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Version Control

By Odin Loch

| | |
|---|---|
| ID | `DV03` (from CATALOGUE.md) |
| Bundle | `org.pbsd.version-control` |
| Category | dev |
| Priority | P2 |
| Backing repos | none (FORGE and FUSE as platform only) |
| Status | Planned. Nothing built. |

## 1. Purpose

A Git client. Developers and writers use it to see what changed, stage and commit with signatures, browse history, branch, merge, rebase and exchange work with remotes and forges. It also provides the repository service that DV01 uses for its version-control panel. It replaces the Git front ends of the Plasma plan.

A repository is untrusted data. Opening one never runs anything it contains: no hooks, no filters, no configured commands.

## 2. Scope

In scope: repository list; status and staging by file, hunk and line; commit with SSH or OpenPGP signing; history graph; branches, tags, remotes, stashes, worktrees, submodules; text, image and structured diff; blame; three-way merge tool; interactive rebase; cherry-pick, revert, bisect; conflict resolution; large files; patches by email; forge pull and merge requests; credentials through the keyring; sandboxed hooks; signature verification.

Out of scope:

| Item | Owner |
|---|---|
| Editing source with language intelligence | DV01 IDE |
| General text editing | OF01 Text Editor |
| Generating and storing SSH and OpenPGP keys, known hosts, trust in signers | SE04 Passwords and Keys |
| Sending email (this app builds the message and hands it over) | CO01 Mail |
| Other version-control systems (Subversion, Mercurial, Fossil) | not planned |
| Hosting repositories or running a forge | not planned |
| Continuous integration runners (this app shows status reported by a forge) | not planned |
| File-level snapshots outside Git | SY02 Files, AD03 Backup |

## 3. Users and top tasks

Personas:

- **Developer**: commits many times a day, rebases, reviews.
- **Writer or designer**: uses Git as a save history; wants safe, plain operations and undo.
- **Maintainer**: reviews pull requests and emailed patches, checks signatures, bisects regressions.
- **Auditor**: inspects an unknown repository without trusting it.

| # | Task | Target steps |
|---|---|---|
| T1 | See what changed since the last commit | 1 (open the repository) |
| T2 | Stage part of a file and commit it signed | 3 (select lines, `S`, `Ctrl+Enter`) |
| T3 | Undo the last commit, keeping the changes | 1 (`Ctrl+Z` in the history view) |
| T4 | Create and switch to a branch | 2 |
| T5 | Fetch and fast-forward | 1 (`Ctrl+Shift+L`) |
| T6 | Resolve a merge conflict | open the conflict, pick sides per hunk, `Ctrl+Enter` |
| T7 | Find who last changed a line and why | 2 (blame, hover) |
| T8 | Reorder and squash the last five commits | drag in the rebase view, Start |
| T9 | Find the commit that broke something | Bisect: mark good and bad, then one key per step |
| T10 | Clone a repository | 3 (address, folder, Clone) |
| T11 | Check that a tag is signed by a known key | 1 (badge on the tag) |
| T12 | Open a pull request for the current branch | 3 |
| T13 | Apply a patch received by email | 2 (open the `.patch` or mbox, Apply) |
| T14 | See what a `.fuseui` change did, by node, not by line | 1 (structured diff tab) |

## 4. Features

Priority: M must, S should, C could. Phase 1, 2, 3 as in section 12.

### 4.1 Repositories

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV03-F001 | Repository list | Sidebar of known repositories with current branch, ahead and behind counts, dirty marker, trust badge. Groups and pinning. | M | 1 |
| DV03-F002 | Add existing | Pick a folder through the portal; the grant is remembered per repository. Bare repositories and worktrees are recognised. | M | 1 |
| DV03-F003 | Open by intent | `open` on a folder that contains `.git`, or on a `.patch`, `.diff`, `.bundle` or mbox file. | M | 1 |
| DV03-F004 | Init | Create a repository in a chosen folder: initial branch name (default `main`), object format (SHA-1 or SHA-256), optional `.gitignore` template and first commit. | M | 1 |
| DV03-F005 | Clone | Address (HTTPS, SSH, local path, bundle file), target folder, branch, depth, single-branch, partial clone filter (`blob:none`, `tree:0`, size limit), sparse paths, submodule policy (none, listed, all), with progress and cancel. | M | 1 |
| DV03-F006 | Clone into a disposable sandbox | "Inspect without keeping": clones into a throw-away dataset, opens read-only, discards on close. | S | 3 |
| DV03-F007 | Repository health | Object count, pack sizes, loose objects, dangling objects, largest blobs; "Maintain" runs pack and prune in the sandbox; integrity check (`fsck`) with results listed. | S | 2 |
| DV03-F008 | Scan a folder | Find repositories under a granted folder (depth limit 4) and add them. | S | 2 |
| DV03-F009 | Remove from list | Removes the entry and the grant; never deletes files. | M | 1 |
| DV03-F010 | Repository settings | User name and email for this repository, default remote, pull strategy (fast-forward only by default), signing key, hooks policy, line-ending and whitespace policy. Written by this app to its own per-repository settings and, where needed for other tools, to `.git/config` after showing the change. | M | 1 |
| DV03-F011 | Identity profiles | Named identities (name, email, signing key reference) picked per repository; a rule can bind an identity to a remote host pattern. | S | 2 |
| DV03-F012 | Multi-window | One window per repository or tabs in one window. | S | 1 |

### 4.2 Repository trust

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV03-F020 | Open runs nothing | Opening and browsing use only the repository reader (section 6), which parses objects, packs, the index and refs. No Git process starts until the user performs an operation that needs one. | M | 1 |
| DV03-F021 | Neutral configuration | Every Git process runs with system and user configuration disabled and a fixed configuration supplied by the app. Repository-level keys that name a program are overridden to nothing: `core.fsmonitor`, `core.pager`, `core.editor`, `core.sshCommand`, `core.hooksPath`, `core.gitProxy`, `diff.external`, `diff.*.textconv`, `diff.*.command`, `filter.*.clean`, `filter.*.smudge`, `filter.*.process`, `merge.*.driver`, `credential.helper`, `gpg.program`, `gpg.ssh.program`, `sequence.editor`, `uploadpack.packObjectsHook`, `sendemail.*`, `alias.*` starting with `!`, `include.path`, `includeIf.*`. | M | 1 |
| DV03-F022 | Configuration review | A view lists every key in `.git/config`, `.gitmodules`, `.gitattributes` and `.lfsconfig` that could run a program, rewrite an address (`url.*.insteadOf`) or change an identity, marked "ignored" or "enabled by you". | M | 1 |
| DV03-F023 | Sandbox as the backstop | The Git process runs in a capability domain that can execute only Git's own program files (passed as descriptors), sees only the repository directory, and has a socket only to the one remote host of the operation, if any. A configuration key the override list missed therefore still cannot start a program. | M | 1 |
| DV03-F024 | Trust levels | **Untrusted** (default): read and local write operations; no hooks, no filters, no submodule update, no LFS smudge. **Trusted hooks and filters**: per item, after review (4.13). Shown as a badge; one click returns to untrusted. | M | 1 |
| DV03-F025 | Dangerous path checks | Refuses checkout of trees containing `.git` in any case or Unicode variant, paths with `..`, absolute paths, symlinks that point outside the worktree when writing through them, names ending in a dot or space, and device names. Lists what was refused. | M | 1 |
| DV03-F026 | Submodule addresses | Submodule URLs are shown before any fetch; local-path and `ext::`/`file://` transports are refused unless the user allows that one. | M | 2 |
| DV03-F027 | Ownership check | A repository owned by another user opens read-only with a banner (the "dubious ownership" case). | M | 1 |
| DV03-F028 | Audit | Each trust change, hook run and remote contact is written to `svc.audit`. | M | 2 |

### 4.3 Status and staging

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV03-F030 | Status list | Two lists, Unstaged and Staged, as a tree or flat: path, status (modified, added, deleted, renamed, copied, type changed, untracked, ignored, conflicted), added and removed line counts. | M | 1 |
| DV03-F031 | Live refresh | Watches the worktree and `.git` (kqueue); refresh is incremental through the index's stat cache; `F5` forces a full scan. | M | 1 |
| DV03-F032 | Stage and unstage file | `S` and `U`, check box, drag between lists; stage all `Ctrl+Shift+A`. | M | 1 |
| DV03-F033 | Stage hunk | Per-hunk Stage, Unstage, Discard buttons in the diff. | M | 1 |
| DV03-F034 | Stage lines | Select lines in the diff and press `S`; the app builds the partial patch. Works on added, removed and changed lines, and on files with no trailing newline. | M | 1 |
| DV03-F035 | Split hunk and edit hunk | Split at unchanged lines; "Edit before staging" opens the hunk for manual edit with validation. | S | 2 |
| DV03-F036 | Discard | File, hunk or lines. Discarded content goes to a local "Discarded changes" store for 30 days and can be restored; the toast offers Undo. | M | 1 |
| DV03-F037 | Untracked files | Add, ignore (write a rule to `.gitignore`, `.git/info/exclude` or the user's ignore list), delete to trash. Folders of untracked files collapse to one row. | M | 1 |
| DV03-F038 | Intent to add | Mark a new file so it shows in diffs without staging content. | C | 3 |
| DV03-F039 | Rename detection | Renames and copies shown as one row with similarity; threshold setting (default 50%). | M | 1 |
| DV03-F040 | Mode and symlink changes | Shown as their own rows; staging them is separate from content. | S | 2 |
| DV03-F041 | Whitespace and line-ending warnings | Flags trailing whitespace, mixed line endings and a missing final newline in added lines; "Fix and restage". | S | 2 |
| DV03-F042 | Secret scan before commit | Added lines are checked against patterns for private keys, tokens and passwords (own rules, local only). A hit blocks the commit until acknowledged per line. | M | 2 |
| DV03-F043 | Large file warning | Files over a threshold (default 10 MiB) or binary files being added raise a warning with choices: add anyway, track as large file (4.12), ignore. | S | 2 |
| DV03-F044 | Assume unchanged and skip worktree | Toggles per file, with a visible marker so they are not forgotten. | C | 3 |
| DV03-F045 | Filter and search the list | By path, status, extension. | S | 1 |
| DV03-F046 | Open file | In DV01, OF01 or the default app (intent); reveal in Files. | M | 1 |

### 4.4 Commit

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV03-F050 | Message editor | Subject and body; subject length guide at 50 and hard mark at 72; body wrap at 72; spell check (G-17); draft kept per repository and branch. | M | 1 |
| DV03-F051 | Commit | `Ctrl+Enter`. Commit and push (`Ctrl+Shift+Enter`). Empty-message and nothing-staged are stated inline, not in a dialog. | M | 1 |
| DV03-F052 | Amend | Toggle that loads the last message and shows the combined diff; warns when the commit is already on a remote. | M | 1 |
| DV03-F053 | Sign commits | SSH or OpenPGP signature made by SE04's signing service. The app passes the commit bytes; the key never enters this app or the Git process. Per repository: always, never, ask. | M | 1 |
| DV03-F054 | Sign-off and trailers | `Signed-off-by`, `Co-authored-by`, `Reviewed-by`, issue references; picker of recent people; custom trailers. | S | 2 |
| DV03-F055 | Author and date override | Author identity and author date fields behind "More". | S | 2 |
| DV03-F056 | Message templates and conventions | Template per repository; optional convention check (for example `type(scope): subject`) with a rule file; failures are warnings. | S | 2 |
| DV03-F057 | Fixup and squash commits | "Fixup for…" picks a target commit and names the commit for autosquash. | S | 2 |
| DV03-F058 | Undo commit | Soft reset of the last commit with one action; the message returns to the editor. | M | 1 |
| DV03-F059 | Commit message from changes (local AI) | Opt-in: drafts a message from the staged diff through `svc.model`; inserted as editable text; never commits by itself. | C | 3 |
| DV03-F060 | Pre-commit summary | Row of checks before the button: signed or not, hooks that will run (if enabled), secret scan, identity in use. | M | 2 |
| DV03-F061 | Commit history of messages | Last 30 messages for reuse. | C | 2 |

### 4.5 History

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV03-F070 | History graph | Commit list with a lane graph, refs as chips (branch, remote branch, tag, HEAD, stash, worktree), author, date, short id, signature badge. Virtualised; draws from the commit-graph file when present. | M | 1 |
| DV03-F071 | Scope | All refs, current branch, chosen refs, first-parent only, a path, a line range of a file (line history), an author. | M | 1 |
| DV03-F072 | Search | By message, author, committer, id prefix, changed path, content added or removed (pickaxe), date range; regex option; results highlight in the graph. | M | 2 |
| DV03-F073 | Commit detail | Full message, author and committer with dates, parents, refs containing it, signature detail, changed files with counts, diff per file or as one scroll. | M | 1 |
| DV03-F074 | Compare | Any two commits, branches or tags: file list and diff; three-dot (since merge base) or two-dot. | M | 2 |
| DV03-F075 | File history | History of one file following renames; side-by-side of any two versions; "open this version" read-only. | M | 2 |
| DV03-F076 | Reflog | Per ref; any entry can be checked out or made a branch. This is the app's general undo source. | M | 2 |
| DV03-F077 | Undo and redo of operations | `Ctrl+Z` undoes the last ref-moving operation (commit, amend, reset, merge, rebase, pull, branch delete) by restoring refs from the operation journal; the worktree is restored when clean. | M | 2 |
| DV03-F078 | Reset | Soft, mixed, hard to a commit. Hard reset first stores the discarded worktree changes in the "Discarded changes" store. | M | 1 |
| DV03-F079 | Copy and export | Copy id, short id, message, patch; "Create patch file", "Create bundle", "Archive this tree" (tar or zip through UT01's helper). | S | 2 |
| DV03-F080 | Graph options | Date order or topological; hide remote branches, tags, stashes; collapse merged branches; relative or absolute dates; column chooser. | S | 2 |
| DV03-F081 | Statistics | Commits per author and over time, churn per file, for the selected range. Local only. | C | 3 |
| DV03-F082 | Notes | Show and edit `refs/notes`. | C | 3 |

### 4.6 Branches, tags, remotes, stashes, worktrees, submodules

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV03-F090 | Branch list | Local and remote trees (slashes as folders), upstream, ahead and behind, last commit date, merged-into-current marker. | M | 1 |
| DV03-F091 | Create, rename, delete | Create from any commit; delete warns about unmerged commits and is undoable (F077); delete on the remote is a separate, confirmed action. | M | 1 |
| DV03-F092 | Checkout | Double click. With local changes: carry them, stash and reapply, or cancel, chosen inline. Detached HEAD is stated in the header with "Create branch here". | M | 1 |
| DV03-F093 | Upstream | Set, change, unset; publish a branch (`push -u`). | M | 1 |
| DV03-F094 | Merge | Into current: fast-forward if possible, no-fast-forward, squash; preview of conflicts before starting (trial merge in memory). | M | 1 |
| DV03-F095 | Tags | Lightweight and annotated; signed tags through SE04; push one or all; delete local and remote; signature badge. | M | 1 |
| DV03-F096 | Remotes | Add, rename, remove, edit fetch and push addresses; prune; per-remote identity and credentials; show which host each contacts. | M | 1 |
| DV03-F097 | Fetch, pull, push | Fetch one or all (`Ctrl+Shift+F`); pull with fast-forward only, merge or rebase (`Ctrl+Shift+L`); push (`Ctrl+Shift+U`), push tags, force-with-lease only (plain force is not offered). Progress, transfer size, cancel. | M | 1 |
| DV03-F098 | Background fetch | Off by default. When on, per repository and interval; needs `bg.run`; shown in the `[net]` badge. | C | 3 |
| DV03-F099 | Stashes | Save (all, staged only, include untracked), named; list with diff; apply, pop, drop (undoable), "branch from stash"; apply single files. | M | 1 |
| DV03-F100 | Worktrees | List, add (new or existing branch, folder through the portal), remove, lock, prune; open a worktree as its own tab. | S | 2 |
| DV03-F101 | Submodules | List with pinned commit, checked-out commit and state; init, update, sync address, add, remove; open as a repository. Update needs the address check of F026. | S | 2 |
| DV03-F102 | Sparse checkout | Edit the cone list; show which folders are present. | S | 3 |
| DV03-F103 | Shallow and partial state | Show depth and filter; "Deepen" and "Fetch missing objects". | S | 3 |
| DV03-F104 | Branch cleanup | Lists merged and stale (no commit for N days, upstream gone) branches for bulk delete. | S | 2 |
| DV03-F105 | Protected branches | Local rule list (default `main`, `master`, `release/*`): commit, force push and delete on these ask first. | S | 2 |

### 4.7 Diff

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV03-F110 | Text diff | Unified and side-by-side; syntax colours from the shared grammars; intra-line word and character highlights; line numbers for both sides. | M | 1 |
| DV03-F111 | Algorithms | Myers, patience, histogram (default); "ignore whitespace" modes (none, at end of line, amount, all), ignore blank lines, ignore case. | M | 1 |
| DV03-F112 | Context | Lines of context 0 to 50 or whole file; expand hidden regions by click; function-name hunk headers from grammar rules. | M | 1 |
| DV03-F113 | Moved code | Detects moved blocks and colours them separately from adds and deletes. | S | 2 |
| DV03-F114 | Invisible characters | Bidirectional controls, zero-width characters and non-ASCII look-alikes in changed lines are shown as chips with a warning (same rule as OF01-F063). | M | 1 |
| DV03-F115 | Image diff | PNG, JPEG, WebP, GIF, SVG (rendered), `.fuseui` golden PNGs: side by side, swipe, onion skin, difference with threshold; size, dimensions and metadata changes listed. Decoding in the image decoder sandbox. | M | 2 |
| DV03-F116 | Structured diff for JSON | Tree diff by key path: added, removed, changed, moved array items (by a key such as `id` when present); ignores key order and whitespace on request. | M | 2 |
| DV03-F117 | Structured diff for `.fuseui` | Node-level diff using node `id`: nodes added, removed, moved to another parent or slot, properties changed, bindings changed, actions changed, style changed; optional rendered before and after (through DV02's preview service). | M | 2 |
| DV03-F118 | Other structured types | Locale tables (`.tbl`) by key; CSV by row and column; manifest `capabilities` as a set with a "this commit adds capabilities" warning. | S | 2 |
| DV03-F119 | Binary diff | Size change and hashes; "Open in Hex Editor" (DV04 binary diff). | M | 2 |
| DV03-F120 | Large diffs | Files over 50 000 changed lines or 16 MiB load on demand; generated files (by attribute or pattern) are collapsed. | M | 1 |
| DV03-F121 | Navigation | Next and previous change `F7`/`Shift+F7`, next and previous file `Alt+Down/Up`, file tree with counts, minimap of changes. | M | 1 |
| DV03-F122 | Copy and export | Copy selection without markers, copy as patch, save as `.patch`. | S | 1 |
| DV03-F123 | Diff of arbitrary files | Compare two files or folders outside any repository. | S | 2 |
| DV03-F124 | Review marks | Mark files as viewed while reading a large change; marks reset when the file changes. | S | 2 |

### 4.8 Blame

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV03-F130 | Blame view | Per line: commit, author, date, in a gutter with age colouring; hover shows the message; click opens the commit. | M | 2 |
| DV03-F131 | Blame options | Ignore whitespace, detect moves within and across files, ignore revisions listed in an ignore file (for mass reformatting commits). | S | 2 |
| DV03-F132 | Blame previous | Step to the blame before the selected commit ("who wrote the line this one replaced"). | S | 2 |
| DV03-F133 | Incremental | Lines fill in as the reader walks history; cancel at any time. | M | 2 |
| DV03-F134 | Blame data service | The same data is served to DV01 for inline blame. | M | 2 |

### 4.9 Merge tool and conflicts

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV03-F140 | Conflict list | Files in conflict with kind (both modified, deleted by us or them, both added, rename conflicts) and count of unresolved hunks. | M | 1 |
| DV03-F141 | Three-way merge tool | Panes: ours, base, theirs on top and the result below (or a three-column layout). Per conflict: take ours, theirs, both (either order), base, or edit the result. Non-conflicting changes are pre-applied and can be reviewed. | M | 2 |
| DV03-F142 | Conflict styles | Reads `merge` and `diff3`/`zdiff3` markers; shows the base. | M | 2 |
| DV03-F143 | Whole-file choices | Use ours or theirs for a file; for delete-modify conflicts: keep or delete. | M | 1 |
| DV03-F144 | Structured merge | For JSON and `.fuseui`: a three-way merge by key path and node id that resolves edits to different nodes automatically and shows only true conflicts. The result is validated (parses; ids unique) before it can be marked resolved. | S | 3 |
| DV03-F145 | Binary conflicts | Pick a side; image conflicts show both with the base. | S | 2 |
| DV03-F146 | Mark resolved, continue, abort | Per file and for the whole operation; the header states which operation is in progress (merge, rebase step n of m, cherry-pick, revert, stash apply) with Continue, Skip, Abort. | M | 1 |
| DV03-F147 | Reuse recorded resolutions | Remembers how a conflict was resolved and proposes it when the same conflict returns during a rebase. | S | 3 |
| DV03-F148 | Leftover markers check | Refuses to mark a text file resolved while it still contains conflict markers, unless confirmed. | M | 1 |
| DV03-F149 | Merge tool for outside files | Open three files and merge them with no repository. | C | 3 |

### 4.10 Rebase, cherry-pick, revert, bisect

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV03-F150 | Rebase | Current branch onto a branch or commit; `--onto` form; preserve merges option; autostash. | M | 2 |
| DV03-F151 | Interactive rebase UI | A list of the commits to replay: drag to reorder; action per row: pick, reword, edit, squash, fixup, drop; insert a pause; autosquash pre-arranges fixups. A preview shows the resulting history. The app writes the instruction list itself; no editor program is started. | M | 2 |
| DV03-F152 | Rebase progress | Step n of m, current commit, conflicts go to the merge tool; Continue, Skip, Abort, "Edit remaining steps". | M | 2 |
| DV03-F153 | Safe rebase | Before starting, records the branch tip in the operation journal; "Undo rebase" restores it. Warns when commits are already on a remote. | M | 2 |
| DV03-F154 | Split a commit | From the interactive list: stop at a commit, reset it, and stage and commit in parts with guidance. | C | 3 |
| DV03-F155 | Cherry-pick | One or many commits, in order, onto the current branch; option to record the source id; conflict flow as above. | M | 2 |
| DV03-F156 | Revert | One or many commits; for a merge commit, choose the parent. | M | 2 |
| DV03-F157 | Bisect | Start with good and bad commits; each step shows the commit under test and the number of steps left; mark good, bad or skip with one key; ends with the first bad commit and a log that can be saved. | M | 2 |
| DV03-F158 | Automated bisect | Runs a user-chosen test command at each step in a sandbox (worktree read-write, no network, time limit). The command comes from the user, never from the repository's configuration. | S | 3 |
| DV03-F159 | Bisect visual | The candidate range is highlighted in the graph and shrinks each step. | S | 3 |

### 4.11 Remotes, credentials and signatures

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV03-F170 | Transports | HTTPS (smart protocol v2), SSH, local path, bundle files. `git://` (unauthenticated) only after a warning. | M | 1 |
| DV03-F171 | Credentials through the keyring | HTTPS passwords and tokens are stored in `svc.keyring` under this app, keyed by host and user. A credential bridge inside the Git sandbox answers Git's credential requests by asking the app, which asks the keyring; the secret is passed once over a pipe and never written to disk or configuration. | M | 1 |
| DV03-F172 | SSH keys | Authentication uses SE04's agent interface; private keys stay there. The SSH client used by Git runs in the same sandbox with a socket to the one host. Host keys are checked against SE04's known-hosts store; a new or changed host key is shown by the shell prompt, with the fingerprint. | M | 1 |
| DV03-F173 | Per-host network grant | A remote host is contacted only after it has a grant (`net.user` first-contact rule): the prompt names the host and port. Grants are listed in AD05 Permissions. | M | 1 |
| DV03-F174 | Route | Fetch and push follow the app's `net.route` (clear, VPN or Tor) set in SE03; the route in use is shown beside the remote. | S | 2 |
| DV03-F175 | Proxy and TLS | Uses the system proxy and trust store; a per-remote pinned certificate fingerprint is possible; TLS verification cannot be turned off globally. | M | 2 |
| DV03-F176 | Signature verification | Commit and tag signatures (SSH and OpenPGP, and X.509 if SE04 supports it) are verified through SE04's verify interface against its trusted signers. | M | 1 |
| DV03-F177 | Signature display | Badge states: **Verified** (valid, signer trusted, `--ok` with icon), **Valid, unknown signer** (`--warn`), **Bad signature** (`--danger`), **Expired or revoked key** (`--warn`), **Unsigned** (neutral). Detail: signer identity, key fingerprint, algorithm, signing time, which trust entry matched. Never colour alone. | M | 1 |
| DV03-F178 | Allowed signers per repository | A repository may require signatures from a set of signers; pulls that bring unsigned or unknown-signer commits to a protected branch are flagged before merge. | S | 2 |
| DV03-F179 | Push certificate and signed push | Signed pushes where the server supports them. | C | 3 |
| DV03-F180 | Mirror and multiple push addresses | Push to several remotes in one action, each with its own result row. | C | 3 |

### 4.12 Large files, patches, forges

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV03-F190 | Large-file pointers | Detects Git LFS pointer files; shows them as large files with size and whether the content is present. | M | 2 |
| DV03-F191 | Large-file transfer | Own client for the LFS batch API over HTTPS (download on checkout on request, upload on push), with locks listed. No filter program from the repository is run. | S | 3 |
| DV03-F192 | Track patterns | Edit which patterns are large files (`.gitattributes`); migrate a file already committed (rewrites history, with the usual warnings). | S | 3 |
| DV03-F193 | Create patches | From commits or a range: one file per commit (`format-patch` layout) with cover letter, version number (`v2`), range-diff against the previous version. | M | 2 |
| DV03-F194 | Send patches by email | Builds the messages (threading, `In-Reply-To`, recipients from trailers and a per-repository list) and hands them to CO01 by `compose` intent, one draft per patch. This app opens no SMTP connection. | S | 2 |
| DV03-F195 | Apply patches | From `.patch`, `.diff` or mbox files, or from CO01 by `open` intent: preview each patch, check that it applies (trial apply), apply as commits keeping author and date, or apply to the worktree only; three-way fallback. | M | 2 |
| DV03-F196 | Bundles | Create and fetch from bundle files for offline transfer. | S | 2 |
| DV03-F197 | Forge accounts | Add an account for a forge host: kind (Forgejo or Gitea API, GitLab API, GitHub REST API, or "generic: none"), host, token (stored in `svc.keyring`). Self-hosted hosts are first-class. Each account's host gets its own network grant. | S | 3 |
| DV03-F198 | Requests list | Pull or merge requests of the repository: title, author, branch, state, review state, check status, labels; filter mine, to review, all. Cached for offline reading. | S | 3 |
| DV03-F199 | Create request | From the current branch: target, title and description (from commits), draft flag, reviewers, labels. | S | 3 |
| DV03-F200 | Review a request | Fetches the request's commits into a local ref; shows the diff with the forge's comments inline; add comments, approve or request changes; check out locally. | S | 3 |
| DV03-F201 | Merge a request | Merge, squash or rebase as the forge allows; shows required checks. | C | 3 |
| DV03-F202 | Issues link | Issue references in messages become links (opened in IN01 by intent); read-only issue title on hover from the cache. | C | 3 |
| DV03-F203 | No account needed | Every non-forge feature works without any account. Forge features are absent, not nagging, when no account exists. | M | 1 |

### 4.13 Hooks and filters

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV03-F210 | Hooks are off by default | Client hooks (`pre-commit`, `prepare-commit-msg`, `commit-msg`, `post-commit`, `pre-push`, `pre-rebase`, `post-checkout`, `post-merge`, `post-rewrite`) do not run until enabled per repository and per hook. | M | 1 |
| DV03-F211 | Hook review | Lists hook files found in `.git/hooks` and any hook directory the repository names, with their content, interpreter line and BLAKE3 hash. Enabling records the hash; a changed file is disabled again until re-reviewed. | M | 2 |
| DV03-F212 | Hook sandbox | An enabled hook runs through the system launcher in a new domain: worktree read-only (read-write only for `pre-commit` when the user allows "may modify files"), `.git` not writable, no network, no keyring, no home directory, CPU, memory and time limits, the interpreter from a system package. Output is shown in the operation log. | M | 2 |
| DV03-F213 | Hook result | Non-zero exit blocks the operation as Git defines; the user can retry or bypass (the bypass is recorded in the operation log, not hidden). | M | 2 |
| DV03-F214 | Filters and custom drivers | Clean, smudge, process filters, `textconv` and custom merge drivers named by the repository are never run from configuration. The app offers built-in equivalents for known cases (line endings, large files, structured merge). Anything else needs an explicit per-driver enable with the same review and sandbox as hooks. | M | 2 |
| DV03-F215 | App-level checks instead of hooks | Built-in pre-commit checks that need no repository code: secret scan, whitespace, conflict markers, large files, message convention. | M | 2 |

### 4.14 Workbench, settings, accessibility, recovery

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| DV03-F220 | Operation log | Every Git operation with the exact arguments, duration, exit status and output; copy as text. Shows the user what the app did. | M | 1 |
| DV03-F221 | Operation journal | Before each ref-moving operation the app records affected refs and HEAD in its store; used by Undo (F077) and crash recovery. | M | 2 |
| DV03-F222 | Interrupted operation recovery | Detects a half-finished merge, rebase, cherry-pick, revert or bisect on open and offers Continue or Abort. A stale `index.lock` is reported with its age and can be removed after a check that no Git process of this app holds it. | M | 1 |
| DV03-F223 | Command palette | `Ctrl+K`: every action, branches (checkout), commits by id or message, files. | M | 1 |
| DV03-F224 | Repository service | IPC interface for DV01 and other apps: status, diff hunks, stage, commit, branch list, blame, change notifications. Callers get only repositories they hold a handle to. | M | 2 |
| DV03-F225 | Notifications | Long operations report through `svc.notify` when the window is not focused. | S | 2 |
| DV03-F226 | Keyboard-first staging | The whole stage-review-commit loop works with single keys in the status and diff views (section 5.5). | M | 1 |
| DV03-F227 | Explain | A plain-language line under risky actions ("This rewrites 5 commits that are already on origin"). | M | 1 |
| DV03-F228 | Beginner mode | Hides rebase, reset and force-with-lease behind "Advanced"; pull is fast-forward only. | S | 2 |
| DV03-F229 | Print | A commit, diff or the log as a document through `svc.portal.print`. | C | 3 |
| DV03-F230 | Search index contribution | Commit messages of listed repositories are offered to `svc.search` (opt-in per repository). | C | 3 |

## 5. GUI

### 5.1 Window map

```
Version Control
├─ Main window
│  ├─ Header bar, toolbar (fetch, pull, push, branch, stash, operation state)
│  ├─ Sidebar: Repositories · (per repository) Changes, History, Branches, Remotes, Tags, Stashes, Worktrees, Submodules, Requests
│  ├─ Content: Changes view │ History view │ Compare view │ Blame view │ Merge tool │ Rebase view │ Bisect view │ Requests view
│  └─ Inspector: commit detail or file detail or signature detail
├─ Dialogs: Clone, Init, New branch, Merge, Rebase onto, Push, Tag, Stash, Remote, Forge account,
│           Create request, Hook review, Configuration review, Create patches, Apply patches, Preferences
├─ Popovers: branch switcher, signature detail, diff options, identity picker
└─ Sheets (narrow): inspector, operation log
```

### 5.2 Main window

Changes view at 1440×900.

```
+------------------------------------------------------------------------------------------+
| [=] Version Control — pbsd-apps      [ Search or run a command  Ctrl+K ]  [net] [▁][▢][✕] | (1)
+------------------------------------------------------------------------------------------+
| [⎇ feature/designer v] ↑2 ↓0  [Fetch] [Pull] [Push]  [Stash] [Branch+]   Trust: Untrusted | (2)
+------------------+------------------------------+----------------------------------------+
| REPOSITORIES     | UNSTAGED (3)        [Stage all]| src/main.fs            [Unified|Split] |
| > pbsd-apps   ●  |  M src/main.fs        +12 −3 | @@ -40,7 +40,9 @@ auto save_note(…)     |
|   forge          |  M ui/editor.fuseui   +4 −1  |  40   auto p = ui::get(pathBox,"text"); |
|   notes          |  ? docs/plan.md              |  41 − fs.write(p, n.body);          (5) |
| ---------------- | STAGED (1)        [Unstage all]|  41 + auto h = portal::save(p);         |
| Changes      (4) |  A style/app.css      +30    |  42 + h.write(n.body);                  |
| History          | (3)                          |  43   ui::toast("Saved");               |
| Branches         +------------------------------+ [Stage hunk] [Discard hunk] [Stage lines]|
| Remotes          | Commit message               | @@ -88,4 +90,4 @@                       |
| Tags             | [Fix save through the portal]|  …                                  (6) |
| Stashes (2)      | [                           ]|                                        |
| Worktrees        | [x] Sign (SSH: odin@pbsd)    |                                        |
| Submodules       | ✓ secrets ✓ whitespace · no hooks                                (7)  |
| Requests         | [ Amend ]        [Commit Ctrl+Enter]                                  |
+------------------+------------------------------+----------------------------------------+
| feature/designer · 3 changed · last fetch 2 h ago · signing on · route: clear        (8) |
+------------------------------------------------------------------------------------------+
```

1. Header bar. `[net]` lights during fetch, pull, push, clone and forge calls; click lists the host.
2. Toolbar: branch switcher (`comboBox` with filter), ahead and behind counts, transfer `toolButton`s, trust chip.
3. Status lists: two `treeView`s with check boxes, status letter and line counts.
4. Sidebar (220 px): repository `listView` on top, section list for the selected repository below.
5. Diff view (FUSE-WP-38): unified or split; selected lines can be staged.
6. Per-hunk action row.
7. Commit box: `lineEdit` subject, multi-line body (shared code editor in plain mode), sign `checkBox`, pre-commit summary row, Amend toggle, primary Commit button.
8. Status bar: branch, counts, last fetch, signing, network route.

States: empty (no repositories: "Add a repository", "Clone", "Create"); loading (status scan progress for large trees; lists fill incrementally); error (repository corrupt: banner with the reader's error and "Run integrity check"); offline (remote actions state "No route to host" inline; all local work is available); locked (session lock does nothing special; credentials are not held in memory beyond one operation); read-only (foreign-owned repository or read-only handle: staging and commit disabled with reason); operation in progress (a bar under the toolbar: "Rebasing 3 of 7" with Continue, Skip, Abort).

### 5.3 Other views and dialogs

#### History view (1440×900)

```
+------------------+-----------------------------------------------+-----------------------+
| (sidebar)        | [All refs v] [search: message, author, path…] | COMMIT a41c9e2        |
|                  | Graph    Message                 Author  Date | Fix save through the  |
|                  | ●─┐  [main][origin/main] Merge…  Odin    2 h ✓| portal                |
|                  | │ ●  [feature/designer] Fix save Odin    3 h ✓|                       |
|                  | │ ●  Add designer palette        Mira    1 d ?| Odin Loch, 3 hours ago|
|                  | ●─┘  Bump schema                 Odin    2 d ✓| ✓ Verified · SSH      |
|                  | ●    [v0.1.0] Release 0.1.0      Odin    9 d ✓|   odin@pbsd  SHA256:…|
|                  | ●    Unsigned import             Sam     9 d —| Parents: 7be2…        |
|                  | (1)                                       (2) | ---------------------|
|                  +-----------------------------------------------+ M src/main.fs  +12 −3 |
|                  | diff of the selected file                 (4) | M ui/editor.fuseui (3)|
+------------------+-----------------------------------------------+-----------------------+
```

1. Lane graph column drawn by a cell delegate (DV03-GAP-1). 2. Signature column: ✓ verified, ? valid but unknown signer, ✕ bad, — unsigned, each with a tooltip word. 3. Changed files `listView` in the inspector. 4. Diff pane. Context menu on a commit: Checkout, Branch here, Tag here, Cherry-pick, Revert, Reset ▸, Rebase onto here, Interactive rebase from here, Fixup for this, Compare with ▸, Create patch, Copy id.

#### Three-way merge tool (1440×900)

```
+------------------------------------------------------------------------------------------+
| Merging origin/main into feature/designer — src/main.fs — conflict 1 of 3   [◀] [▶]      |
+----------------------------+----------------------------+--------------------------------+
| OURS (feature/designer)    | BASE                       | THEIRS (origin/main)           |
| 41 auto h = portal::save(p)| 41 fs.write(p, n.body);    | 41 fs.write_atomic(p, n.body); |
| 42 h.write(n.body);        |                            |                                |
| [Take ours]                | [Take base]                | [Take theirs]  [Both ▾]        |
+----------------------------+----------------------------+--------------------------------+
| RESULT (editable)                                                                        |
| 40   auto p = ui::get(pathBox, "text");                                                  |
| 41 ▌ auto h = portal::save(p);            ← from ours                                (1) |
| 42 ▌ h.write(n.body);                                                                    |
| 43   ui::toast("Saved");                                                                 |
+------------------------------------------------------------------------------------------+
| 2 unresolved · 5 auto-merged [Review]      [Abort merge]  [Mark resolved]  [Next file]   |
+------------------------------------------------------------------------------------------+
```

1. Result lines carry a source tag. Shortcuts: `1` ours, `2` theirs, `3` both, `0` base, `F7`/`Shift+F7` next and previous conflict. States: all resolved (primary button becomes "Mark resolved and continue"); markers left (warning strip).

#### Interactive rebase (900×600)

```
+----------------------------------------------------------------------------+
| Rebase feature/designer onto origin/main — 5 commits                        |
+------+----------+---------+--------------------------------------+---------+
| ⠿    | Action   | Id      | Subject                              | Signed  |
| ⠿    | [pick v] | a41c9e2 | Fix save through the portal          | ✓       |
| ⠿    | [squash v]| 9d0e11a | fixup! Fix save through the portal   | ✓       |
| ⠿    | [reword v]| 3c7a802 | Add desinger palette                 | ✓       |
| ⠿    | [drop v] | 1f2b3c4 | debug prints                         | —       |
| ⠿    | [pick v] | 77aa001 | Grid layout tool                     | ✓       |
+------+----------+---------+--------------------------------------+---------+
| Result: 3 commits. Signatures will be re-made with odin@pbsd.               |
| ⚠ 2 of these commits are already on origin/feature/designer.                |
|                    [Cancel]  [Reset list]  [Start rebase]                   |
+----------------------------------------------------------------------------+
```

`tableView` with drag handles and a `comboBox` delegate. Keys: `P`, `R`, `E`, `S`, `F`, `D` set the action; `Alt+Up/Down` moves the row.

#### Clone dialog (640×460)

```
+--------------------------------------------------------------+
| Clone a repository                                            |
| Address   [ssh://git@code.example/pbsd/apps.git           ]   |
|           Host: code.example · port 22 · SSH · new host   (1) |
| Folder    [~/src/apps                         ] [Pick…]       |
| Branch    [default v]    [ ] Single branch                    |
| History   (•) Full  ( ) Depth [ 1 ]                           |
| Files     (•) All   ( ) Blobs on demand  ( ) Sparse [paths…]  |
| Submodules (•) Do not fetch  ( ) Ask per submodule            |
| After clone: hooks and filters stay off until you review them.|
|                                   [Cancel]  [Clone]           |
+--------------------------------------------------------------+
```

1. Parsed address summary; a new host means the shell will show a network prompt and a host-key prompt. States: progress (objects, bytes, rate, cancel), failed (reason, "Show log").

#### Hook and configuration review (760×520)

```
+--------------------------------------------------------------------------+
| What this repository asks to run                                          |
+----------------------+----------+----------------------------------------+
| Item                 | State    | Detail                                 |
| hook pre-commit      | [Off v]  | 42 lines, sh, hash b3:77c1…  [View]    |
| hook pre-push        | [Off v]  | 8 lines, sh                  [View]    |
| filter.lfs.process   | Ignored  | built-in large-file support used       |
| core.fsmonitor       | Ignored  | "…/watch.sh"  ⚠ names a program        |
| url.insteadOf        | Ignored  | rewrites https://a → ssh://b           |
| merge.ours.driver    | [Off v]  | "true"                                 |
+----------------------+----------+----------------------------------------+
| Enabled items run in a sandbox: no network, no home folder, worktree      |
| read-only unless you allow changes.          [Close]  [Apply]             |
+--------------------------------------------------------------------------+
```

"View" opens the file read-only with highlighting. Enabling shows what the sandbox allows.

#### Signature detail popover (380×220)

```
+------------------------------------------+
| ✓ Verified                                |
| Signer   Odin Loch <odin@pbsd.example>    |
| Key      SSH ed25519 SHA256:AbC…xyz       |
| Trusted  by "Project signers" in Keys     |
| Signed   2026-11-12 10:41 +11:00          |
| [Open in Passwords and Keys]  [Copy]      |
+------------------------------------------+
```

Other states replace the first line: "? Valid signature, unknown signer" with "Trust this signer…" (opens SE04), "✕ Bad signature: content does not match", "Key expired on …", "Unsigned".

#### Structured `.fuseui` diff (content pane)

```
+------------------------------------------------------------------------------+
| ui/editor.fuseui   [Text] [Structure] [Rendered]                              |
| v panel #root                                                                 |
|   ~ lineedit #title      text: "{note.title}" → "{=note.title}"   (binding)   |
|   + button #save         text "Save", on.clicked "save"                       |
|   ↕ label #status        moved: #root[3] → #footer[0]                         |
|   − separator #sep2                                                           |
+------------------------------------------------------------------------------+
```

`treeView` with change glyphs (`+ − ~ ↕`) and words in tooltips; "Rendered" shows before and after images from DV02's preview service.

### 5.4 Menus

| Menu | Items |
|---|---|
| File | Add Repository… (`Ctrl+O`) · Clone… (`Ctrl+Shift+O`) · New Repository… (`Ctrl+N`) · Open Patch or Bundle… · Compare Files… · Repository Settings… · Configuration and Hook Review… · Print… (`Ctrl+P`) · Close Tab (`Ctrl+W`) · Quit (`Ctrl+Q`) |
| Edit | Undo Operation (`Ctrl+Z`) · Redo Operation (`Ctrl+Shift+Z`) · Copy (`Ctrl+C`) · Copy Commit Id (`Ctrl+Shift+C`) · Find (`Ctrl+F`) · Find in History (`Ctrl+Shift+H`) · Preferences (`Ctrl+,`) |
| View | Command Palette (`Ctrl+K`) · Changes (`Ctrl+1`) · History (`Ctrl+2`) · Branches (`Ctrl+3`) · Stashes (`Ctrl+4`) · Requests (`Ctrl+5`) · Unified or Split Diff (`Ctrl+Shift+D`) · Whitespace ▸ · Context Lines ▸ · Operation Log (`Ctrl+Shift+G`) · Sidebar (`F9`) · Inspector (`F4`) · Refresh (`F5`) · Zoom (`Ctrl++/-/0`) · Full Screen (`F11`) |
| Changes | Stage (`S`) · Unstage (`U`) · Stage All (`Ctrl+Shift+A`) · Discard… (`Delete`) · Stash… (`Ctrl+Shift+S`) · Ignore… · Commit (`Ctrl+Enter`) · Commit and Push (`Ctrl+Shift+Enter`) · Amend Last Commit · Undo Last Commit |
| Branch | New Branch… (`Ctrl+B`) · Switch… (`Ctrl+Shift+B`) · Rename… (`F2`) · Delete… · Merge into Current… (`Ctrl+M`) · Rebase Current onto… (`Ctrl+R`) · Interactive Rebase… (`Ctrl+Shift+R`) · Set Upstream… · New Tag… (`Ctrl+T`) · Worktree ▸ · Clean Up Branches… |
| Commit | Checkout · Cherry-pick (`Ctrl+Shift+P`) · Revert · Reset ▸ Soft, Mixed, Hard · Fixup for This… · Compare with Working Tree · Compare with… · Blame File (`Ctrl+Shift+M`) · File History · Create Patch… · Bisect ▸ Start…, Good (`G`), Bad (`B`), Skip (`K`), Reset |
| Remote | Fetch (`Ctrl+Shift+F`) · Fetch All · Pull (`Ctrl+Shift+L`) · Push (`Ctrl+Shift+U`) · Push Tags · Force with Lease… · Manage Remotes… · Forge Accounts… · Create Request… · Send Patches by Email… · Apply Patches… |
| Help | Handbook (`F1`) · Keyboard Shortcuts · What Did the App Run? (operation log) · About |

### 5.5 Keyboard and pointer

- Status list: `Up/Down` move, `Space` toggles staged, `S`/`U`, `Enter` focuses the diff, `Delete` discards (undoable), `I` ignore, `O` open file.
- Diff: `J`/`K` next and previous hunk, `S` stage hunk or selected lines, `U` unstage, `X` discard, `F7`/`Shift+F7` next and previous change, `Tab` returns to the list, `C` focuses the commit message.
- History: `/` search, `Enter` open detail, `B` branch here, `T` tag here, `Y` copy id.
- Context menus: status rows (Stage, Unstage, Discard, Stash This File, Ignore, Open, Reveal, File History, Blame); branches (Checkout, Merge, Rebase onto, Rename, Delete, Push, Set Upstream, Compare, Create Request); stashes (Apply, Pop, Drop, Branch from); remotes (Fetch, Prune, Edit, Remove); diff lines (Stage, Discard, Copy, Blame This Line, Open at Line).
- Drag and drop: files between Unstaged and Staged; a branch onto another (menu: merge, rebase onto); a commit onto a branch (cherry-pick); a tag chip to a commit; a folder from Files onto the repository list (add); a `.patch` onto the window (apply preview); rows in the rebase list (reorder).

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | Three tiles: Add, Clone, Create; a line "Opening a repository runs nothing from it." |
| Loading | Lists fill incrementally; graph draws the first 200 commits at once and continues in the background. |
| Error | Corrupt object or index: banner with object id and "Integrity check"; other repositories unaffected. Git process failure: operation bar turns `--danger` with "Show log". |
| Offline | Local operations unaffected; remote buttons stay enabled and fail fast with the reason; request lists show cached data with its age. |
| Locked | No special state. |
| Read-only | Foreign-owned or read-only handle: write actions disabled with tooltips. |
| Operation in progress | Bar with step, Continue, Skip, Abort; conflicting actions disabled with reason. |
| Narrow (< 640 px) | Sidebar overlay; status list and diff stack vertically; inspector as a sheet; merge tool switches to tabs (Ours, Base, Theirs, Result). |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Repository list, sections | `listView`, `treeView` |
| Status lists, branches, structured diff | `treeView` with `checkBox` rows |
| History | `tableView` with a lane-graph cell delegate (`setTableDelegate`) |
| Diff, blame, merge panes | diff and merge view (FUSE-WP-38) on the code editor engine (FUSE-WP-37) |
| Commit message | `lineEdit` + code editor in plain mode |
| Rebase list, hook review, requests | `tableView` with `comboBox` delegates |
| Image diff | image compare (DV03-GAP-2) |
| Toolbar, dialogs, menus | `toolButton`, `comboBox`, `dialog`, `menuBar`, `popupMenu`, `radioButton`, `checkBox`, `spinBox`, `progressBar`, `tooltip` |
| Request description and comments | `markdownView` |
| Statistics | `chart` |

| ID | Missing | Note |
|---|---|---|
| DV03-GAP-1 | Lane graph drawing in a table cell at 60 Hz for 1 million rows | Delegate exists (Verified: `setTableDelegate`); needs the 64-bit virtualisation of FUSE-WP-28 |
| DV03-GAP-2 | Image compare widget (side by side, swipe, onion, difference) | Shared with DV01 and DV02 (DV02-GAP-4) |
| DV03-GAP-3 | Drag-reorder rows in `tableView` | Needed for the rebase list |
| DV03-GAP-4 | Per-line gutter annotations and inline action rows in the diff view | Requirement on FUSE-WP-38 |

## 6. Architecture

### 6.1 Components

#### Git implementation plan

Three options were weighed.

| Option | For | Against |
|---|---|---|
| A. Own implementation of everything in safe-subset C++23 | Memory safe, provable parsers, no foreign process, exact control of what can run (nothing) | Years of work to reach parity: transports, pack negotiation, rebase machinery, partial clone, every on-disk format variant. Risk of subtle incompatibility that corrupts repositories |
| B. libgit2 (GPL-2.0 with a linking exception) | A library, no process to confine; does not run hooks or filter programs by itself | It is C, outside the safe subset, and parses hostile data, so it would have to sit in a sandboxed helper anyway. Behind upstream Git on several features (interactive rebase machinery, some partial-clone and maintenance functions, newer protocol and format work). Adds a large unverified C dependency for less function than option C. The linking exception makes the licence workable, but nothing else favours it |
| C. The upstream `git` program (GPL-2.0) run as a separate process in a sandbox | Complete and exact behaviour; maintained by upstream; a separate program, so the licence does not reach the app | It is C. It reads repository configuration that can name programs. Start-up cost per call. Output must be parsed |

**Recommendation: a hybrid, staged from C towards A. Do not use libgit2.**

1. **Reading is own code from the start.** A repository reader in safe-subset C++23 (helper `repo-reader`) parses loose objects, packfiles and their indexes, multi-pack index, commit-graph, the index file, refs, packed-refs, reftable, reflogs, `.gitattributes`, `.gitignore` and configuration files (as data). It serves status, history, graph, diff input, blame and search. This is the code that touches hostile bytes on open, it executes nothing, and it is the part that can reach proof tier T2 or T3. Opening and browsing a repository therefore never starts Git.
2. **Writing and transport start with upstream Git in a sandbox** (helper role `git-runner`, the program itself is a system package). Commit, merge, rebase, fetch, push and clone call plumbing and porcelain commands with machine-readable output (`--porcelain=v2`, `-z`). Safety rests on two layers: the neutral configuration (F021) and the sandbox that can execute nothing but Git's own program files and sees only the repository and one socket (F023).
3. **Own write paths replace Git step by step**, simplest first: index write and staging, commit and tag object creation, ref updates, stash, then merge and rebase. Each replacement is differential-tested against upstream Git on a corpus before it ships. Transport (smart HTTP and SSH, protocol v2) is last.
4. Signing and verification never use Git's `gpg` path: the app builds the object, asks SE04 to sign, and writes the signed object itself (through `git hash-object -w` in stage 2, own code in stage 3).

Third-party code needed: `git` (GPL-2.0, separate program); zlib-format inflate and deflate (own code, or zlib under the zlib licence inside `repo-reader`); SHA-1 with collision detection (own code following the public sha1collisiondetection method, or that library under MIT) and SHA-256; an SSH client (system service or the base-system client, BSD licence, in the sandbox).

#### Component table

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App shell and view models | ForgeScript + FUSE UI | app host | Windows, lists, operation orchestration, settings, journal, undo |
| Operation engine | ForgeScript | app host | Builds each operation as a list of steps (reader queries, runner calls), records the journal, parses runner output |
| `repo-reader` | C++23 | bundle helper; repository directory read-only; no network | Parses all on-disk Git formats; answers status, log, graph layout, tree and blob reads, blame, pickaxe search; never writes |
| `diff-helper` | C++23 | bundle helper; no capabilities (content arrives as shared-memory handles) | Text diff algorithms, intra-line diff, move detection, rename similarity, three-way text merge, patch build and apply, mbox and patch parsing |
| Structured diff module | ForgeScript | app host | JSON, `.fuseui`, `.tbl`, CSV, manifest diffs and three-way structured merge on parsed values |
| Git runner | upstream `git` (system package) | sandbox per operation, started through the launcher (`proc.exec`) | Write operations and transport in stage 2 |
| Credential bridge | C++23 (tiny) | inside the runner sandbox, pipe to the app host | Answers Git's credential protocol from the keyring through the app |
| Signing bridge | ForgeScript | app host | Sends bytes to SE04 for signing and verification |
| Forge client | ForgeScript | app host | REST calls to forge hosts with per-host grants; JSON parsed with size and depth caps |
| Large-file client | ForgeScript + `diff-helper` hashing | app host | LFS batch API |
| Repository service | ForgeScript | app host | IPC for DV01 |

```
 user ─ FUSE UI ─ [ app host: shell · operation engine · structured diff · forge client · service ]
                     │            │                 │                │                │
                     │      repo-reader        diff-helper     system launcher    session bus
                     │   (repo dir, read-only)  (no caps)           │            SE04 sign/verify
                     │                                              ▼            svc.keyring
                     │                               [ sandbox: git + credential bridge + ssh ]
                     │                                 repo dir rw · exec: git files only
                     └─ net (forge REST, LFS) ─ granted hosts        socket: one remote host
```

### 6.2 Data model

App store `~/.local/share/org.pbsd.version-control/vc.db` (SQLite, `user_version` = schema):

| Table | Fields |
|---|---|
| `repo` | `id TEXT PK`, `name`, `grant_id`, `group_name`, `pinned INT`, `trust INT`, `identity_id`, `sign_mode INT`, `pull_mode INT`, `last_opened INT` |
| `identity` | `id`, `name`, `email`, `key_ref TEXT` (SE04 item id, never key material), `key_kind TEXT` |
| `enabled_item` | `repo_id`, `kind TEXT` (hook, filter, driver), `name`, `hash BLOB(32)`, `may_write INT`, `enabled_at INT` |
| `journal` | `id`, `repo_id`, `ts`, `op TEXT`, `args TEXT`, `refs_before TEXT` (JSON map ref → id), `head_before TEXT`, `refs_after TEXT`, `undone INT` |
| `discarded` | `id`, `repo_id`, `ts`, `path`, `patch BLOB` (zstd), `expires INT` |
| `draft_message` | `repo_id`, `branch`, `subject`, `body` |
| `resolution` | `repo_id`, `conflict_hash BLOB(32)`, `result BLOB` |
| `forge_account` | `id`, `kind`, `host`, `user`, `keyring_ref` |
| `request_cache` | `repo_id`, `number`, `json BLOB`, `fetched_at` |
| `viewed` | `repo_id`, `commit_range`, `path`, `blob_id` |
| `oplog` | `id`, `repo_id`, `ts`, `argv TEXT`, `exit INT`, `ms INT`, `output BLOB` |

Cache `~/.cache/org.pbsd.version-control/<repo>/`: graph layout, blame cache, rename cache, thumbnails for image diff. Nothing is written into the repository except by Git operations the user asked for.

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| Loose objects, packfile v2, pack index v2, multi-pack index, commit-graph, bitmap index (skip), index v2 to v4, refs, packed-refs, reftable, reflog | yes | stage 2: by `git`; stage 3: own | own code (`repo-reader`) | `repo-reader` |
| `.git/config`, `.gitmodules`, `.gitattributes`, `.gitignore`, `.lfsconfig` | yes (as data) | selected keys | own | `repo-reader` |
| Git porcelain v2 and plumbing output | yes | — | own | app host (line and size caps) |
| Smart HTTP v2, SSH transport, bundle v2 and v3 | by `git` | by `git` | upstream `git` | runner sandbox |
| Unified diff, `format-patch`, mbox | yes | yes | own | `diff-helper` |
| SSH signatures (`SSHSIG`), OpenPGP signatures | verify through SE04 | sign through SE04 | SE04 | SE04's helper |
| Images for image diff | yes | no | platform decoders | decoder sandbox |
| JSON, `.fuseui`, `.tbl`, CSV | yes | merge result | own (ForgeScript, strict, capped) | app host |
| Forge REST APIs (JSON over HTTPS) | yes | yes | own client on the platform HTTP facility | app host; JSON capped at 8 MiB, depth 64 |
| LFS pointer file, LFS batch API | yes | yes | own | app host |
| tar, zip (archive export) | no | yes | UT01's archive helper | that helper |

### 6.4 Integration

- Intents handled: `open` (folder with `.git`, `.patch`, `.diff`, `.bundle`, mbox), `view` (diff of two files). Emitted: `open` and `edit` (files to DV01, OF01), `compose` (CO01, patches), `open` (DV04 binary diff, IN01 links, SE04 key or signer), `share`, `print`.
- Services: `svc.settings`, `svc.notify`, `svc.portal.file`, `svc.portal.print`, `svc.intent`, `svc.keyring`, `svc.clipboard`, `svc.accounts` (forge endpoints), `svc.audit`, `svc.undo-trash`, `svc.search` (opt-in), `svc.model` (opt-in).
- IPC offered: `org.pbsd.version-control.Repository/1` — `status(handle)`, `hunks(handle, path)`, `stage(handle, path, ranges)`, `commit(handle, message, options)`, `branches`, `checkout`, `blame(handle, path)`, `changed` signal. The caller passes its own directory handle; the service acts only inside it.
- SE04: sign, verify, agent, known hosts, trusted signers. DV02: rendered before and after for `.fuseui`. DV01: client of the repository service. CO01: patch sending and receiving.
- Backing repos: none. Everything here is Proposed.

## 7. Security and privacy

| Capability | Reason |
|---|---|
| `fs.app` | Store, caches, discarded-changes store |
| `fs.portal` | Repository folders the user adds; grants remembered per repository |
| `proc.helper:repo-reader`, `proc.helper:diff-helper` | Bundle helpers |
| `proc.exec` | Run the system `git` program, enabled hooks and bisect test commands, each in its own sandbox through the launcher |
| `net.user` | Remote hosts the user types (clone, fetch, push, forge and large-file hosts); each first contact is prompted and logged; each host becomes a named grant |
| `svc.keyring` | HTTPS credentials and forge tokens; SSH and signing through SE04's interfaces |
| `svc.accounts` | Forge endpoints the user added |
| `svc.settings`, `svc.notify`, `svc.portal.file`, `svc.portal.print`, `svc.intent`, `svc.clipboard`, `svc.audit`, `svc.undo-trash` | As in 6.4 |
| `svc.search`, `svc.model`, `ai.local` | Optional, off by default |
| `bg.run` | Only if background fetch is turned on; off by default |

`net.none` is not declared because remotes are the point of the app; with no remote operation in progress the app holds no socket.

Assets: source code; credentials and tokens; signing ability; the integrity of history; the rest of the user's files.

Adversaries: a malicious repository (objects, index, configuration, hooks, attributes, submodules, paths); a malicious remote server; a malicious patch or mbox; a malicious forge response; a network attacker; a contributor trying to pass off unsigned or forged commits.

| Threat | Mitigation |
|---|---|
| Code execution on open through hooks, `fsmonitor`, filters, pagers, aliases, includes | F020 to F023: reader-only open, neutral configuration, exec-limited sandbox |
| Memory corruption in a parser from crafted packs or index | Parsing in `repo-reader` (safe subset, read-only handle, fuzzed). Git's own C parsers run only in the runner sandbox with access to that one repository |
| Path tricks on checkout (`.git` variants, `..`, symlink escape, case folding) | F025; all writes are beneath the repository directory handle (`openat` beneath) |
| Submodule or remote address abuse (`ext::`, local paths, option injection with a leading `-`) | F026; addresses are parsed and validated by the app; arguments are passed after `--` |
| Credential theft by a repository or server | Secrets never in configuration or environment; passed once over a pipe to Git in a sandbox that reaches only that host; credentials are keyed by exact host and never sent after a cross-host redirect |
| Malicious server sending hostile packs | Received in the runner sandbox; `fsck` on fetch is on; objects are re-read by `repo-reader` |
| Hook exfiltrates data | Hooks are off by default and, when on, have no network, keyring or home access |
| Forged authorship | Signature verification and clear display (F176, F177); author and committer both shown; unknown signers never shown as verified |
| Trojan source in diffs | Invisible and bidirectional characters shown as chips (F114) |
| Forge response injection (HTML or script in descriptions) | Rendered by `markdownView` as Markdown only; no HTML, no remote images |
| Destructive mistakes | Journal and undo (F077), discarded-changes store, force-with-lease only, protected branches |
| Secrets committed by accident | Secret scan (F042) |
| Tracking through background fetch | Off by default; route shown; each contact in `svc.audit` |

Stored: section 6.2. Leaves the machine: Git traffic to the remotes the user operates on; forge and large-file API calls to hosts with grants; nothing else.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `identity.default` | ref | none | Default identity |
| `commit.sign` | enum | `ask` | `always`, `never`, `ask` |
| `commit.subject_limit` | int | 72 | Hard mark |
| `commit.secret_scan` | bool | true | Scan added lines |
| `commit.large_file_bytes` | int | 10 MiB | Warning threshold |
| `pull.mode` | enum | `ff-only` | `ff-only`, `merge`, `rebase` |
| `push.force` | enum | `with-lease` | Only value offered; setting exists to document it |
| `fetch.prune` | bool | true | Prune on fetch |
| `fetch.background` | bool | false | Background fetch |
| `fetch.interval_min` | int | 30 | When background fetch is on |
| `fetch.fsck` | bool | true | Check received objects |
| `diff.algorithm` | enum | `histogram` | `myers`, `patience`, `histogram` |
| `diff.view` | enum | `unified` | `unified`, `split` |
| `diff.context` | int | 3 | Context lines |
| `diff.whitespace` | enum | `show` | `show`, `eol`, `amount`, `all` |
| `diff.rename_threshold` | int | 50 | Percent |
| `diff.structured_default` | bool | true | Open `.fuseui` and JSON in structure view |
| `history.order` | enum | `date` | `date`, `topo` |
| `history.dates` | enum | `relative` | `relative`, `absolute` |
| `hooks.default` | enum | `off` | Cannot be set to on globally |
| `hooks.time_s` | int | 120 | Hook time limit |
| `trust.foreign_owner` | enum | `read-only` | Behaviour for repositories owned by another user |
| `discarded.keep_days` | int | 30 | Discarded-changes retention |
| `protected.branches` | string[] | `main, master, release/*` | Ask before rewriting |
| `mode.beginner` | bool | false | Hide advanced actions |
| `ai.commit_message` | bool | false | Local draft of commit messages |
| `git.package` | string | `git` | Which installed Git package the runner uses |

## 9. Accessibility and languages

- The graph is also exposed as text: each row announces "commit, subject, author, date, signature state, branches, n parents".
- Diff lines announce "added", "removed" or "unchanged" before the text; a setting reads only changed lines.
- Change glyphs and signature states always have a word; added and removed lines differ by a gutter sign as well as colour, with a colour-blind-safe pair available.
- Commit messages are not translated. Dates follow the locale. Right-to-left UI mirrors panes; code and diffs stay left-to-right per line with UAX 9.
- All staging and merge actions have single-key shortcuts and are in the command palette.

## 10. Performance budgets

| Metric | Budget |
|---|---|
| Cold start to first frame | ≤ 300 ms |
| Status of a 100 000-file worktree, warm | ≤ 300 ms; incremental update after one file change ≤ 30 ms |
| History: first 200 commits drawn | ≤ 150 ms on a 1 million commit repository with a commit-graph file |
| Scroll the graph | 60 Hz |
| Diff of a 10 000-line file | ≤ 50 ms |
| Blame of a 5 000-line file with 10 000 commits of history | first lines ≤ 300 ms, complete ≤ 5 s |
| Git process start overhead (sandbox plus exec) | ≤ 40 ms per call; operations batch calls where possible |
| Memory, app host | ≤ 200 MiB; `repo-reader` ≤ 512 MiB with memory-mapped packs |
| Largest repository | 10 million objects, 50 GiB of packs, 1 million refs |
| Idle | zero frames and wake-ups except file-watch events |

## 11. Testing and proof

Test plan:

- Unit (ForgeScript): operation engine step lists, porcelain parsers, journal and undo, structured diff and merge, forge response handling, configuration neutraliser.
- Differential against upstream Git: for a corpus of repositories (generated and real), `repo-reader` results must equal `git` plumbing output for status, log order, tree contents, blame and rename detection; line staging must produce the same index as `git apply --cached` on the same patch; stage 3 writers must produce byte-identical objects.
- Hostile repository suite: fixtures for every known class (hooks, fsmonitor, filters, aliases, includes, path tricks, submodule addresses, oversized objects, delta bombs, cyclic deltas, bad checksums). Opening and every non-enabled operation must execute nothing; a canary program records any execution.
- Fuzz: every `repo-reader` parser, patch and mbox parsers, porcelain parsers, forge JSON, LFS pointers.
- Golden renders: all main views and dialogs, three themes.
- Scripted UI: the 14 top tasks; a full merge-conflict and interactive-rebase flow.
- Crash tests: kill the app and the runner at each step of merge, rebase and commit; the repository must be consistent and recoverable.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| `repo-reader` object and pack parsers | No runtime errors on any input; bounded memory per object; delta chains bounded | T2 | PRISM + fuzz | Full functional correctness needs a format model |
| `repo-reader` index and ref parsers | Same | T2 | PRISM + fuzz | — |
| Configuration neutraliser | For every key in the dangerous list the effective value passed to Git is empty | T1 | Small pure function; PRISM proof of the list property | The list itself can be incomplete; the sandbox covers that |
| Checkout path validator | No accepted path resolves outside the worktree or into `.git` | T1 | Lean model of path normalisation; PRISM on the code | Case-folding rules of the file system must be fixed |
| `diff-helper` | No runtime errors; patch apply is the inverse of patch build | T2 (safety), T3 (inverse property by differential test) | PRISM + fuzz + differential | Proof of diff optimality not attempted |
| Operation journal and undo | Undo restores exactly the recorded refs | T3 | Property tests | ForgeScript module |
| Upstream `git` | none claimed | T4 | Confined | External C code |

Nothing here has been run.

## 12. Build plan

Phase 1: usable core.

| ID | Work item | Depends on |
|---|---|---|
| DV03-W01 | Shell: repository list, sections, command palette | G-01, G-02, G-07 |
| DV03-W02 | `repo-reader`: objects, packs, index, refs, config as data | FORGE-WP-28.1 |
| DV03-W03 | Status and history views on the reader; graph layout | DV03-W02, DV03-GAP-1, FUSE-WP-28 |
| DV03-W04 | `diff-helper`: text diff, rename detection, patch build | — |
| DV03-W05 | Diff view with hunk and line staging | FUSE-WP-38, DV03-W04, DV03-GAP-4 |
| DV03-W06 | Git runner: sandbox profile, neutral configuration, porcelain parsers, operation log | FORGE-WP-27.5, G-06, system `git` package (SY05) |
| DV03-W07 | Commit, amend, branch, checkout, merge (fast-forward and simple), stash, tag, reset | DV03-W06 |
| DV03-W08 | Clone, fetch, pull, push over HTTPS and SSH; credential bridge; host grants | DV03-W06, G-09 (`svc.keyring`), SE04, SSH client (section 13) |
| DV03-W09 | Signing and verification through SE04; signature badges | SE04 |
| DV03-W10 | Trust model: configuration review, dangerous path checks, ownership check | DV03-W02 |
| DV03-W11 | Conflict list, whole-file choices, interrupted operation recovery | DV03-W07 |

Phase 2: full feature.

| ID | Work item | Depends on |
|---|---|---|
| DV03-W20 | Three-way merge tool | FUSE-WP-38, DV03-W04 |
| DV03-W21 | Rebase and interactive rebase UI, cherry-pick, revert | DV03-W07, DV03-GAP-3 |
| DV03-W22 | Operation journal, undo and redo, discarded-changes store | DV03-W07 |
| DV03-W23 | Blame, file history, search, compare, reflog | DV03-W02 |
| DV03-W24 | Image diff and structured diff (JSON, `.fuseui`, tables, manifest) | DV03-GAP-2, DV02 preview service |
| DV03-W25 | Bisect | DV03-W07 |
| DV03-W26 | Hooks and drivers: review, enable, sandbox | DV03-W06, DV03-W10 |
| DV03-W27 | Patches: create, apply, email hand-off; bundles | DV03-W04, CO01 |
| DV03-W28 | Worktrees, submodules, large-file pointers, repository health | DV03-W06 |
| DV03-W29 | Repository service for DV01 | G-08, DV03-W05 |
| DV03-W30 | Secret scan and built-in pre-commit checks | DV03-W05 |

Phase 3: advanced.

| ID | Work item | Depends on |
|---|---|---|
| DV03-W40 | Own write paths: index, commit and tag objects, refs, stash (differential-tested) | DV03-W02, DV03-W22 |
| DV03-W41 | Own merge and rebase engine | DV03-W40, DV03-W20 |
| DV03-W42 | Forge accounts, requests list, create, review, merge | G-09 (`svc.accounts`), platform HTTP facility |
| DV03-W43 | Large-file transfer client, track patterns | DV03-W28 |
| DV03-W44 | Structured three-way merge; recorded resolutions | DV03-W24 |
| DV03-W45 | Sparse and partial clone controls; disposable clone; automated bisect | DV03-W08 |
| DV03-W46 | Own transport (smart HTTP v2, then SSH) | DV03-W41 |
| DV03-W47 | Local AI commit message draft; statistics; search contribution | G-15 |

## 13. Open questions and risks

1. **Exec-limited sandbox.** The design needs a domain in which `git` can run its own sub-programs but nothing else. Under Capsicum that means passing descriptors for each program file and using `fexecve`; upstream Git executes helpers by path. A small launcher shim or a patched build of the Git package is needed. This is the main technical risk of stage 2.
2. **Dynamic host grants.** The manifest can only say `net.user`. A typed remote host should become a visible, revocable `net.host` grant at run time. PLATFORM.md section 6 does not say how a run-time grant is recorded for hosts; FORGE-WP-08.5 may cover it.
3. **SSH client.** Git over SSH needs a client inside the sandbox that talks to SE04's agent. A shared system SSH client service would serve DV01, DV03, DV05, SY03, VM04 and IN03. Not in PLATFORM.md section 8.
4. **SE04 interfaces.** Sign, verify, agent and known-hosts interfaces are assumed from the catalogue line for SE04; its spec was not read.
5. **SHA-1.** Git's default object format is still SHA-1. The reader must use collision detection. SHA-256 repositories are supported but few servers accept them.
6. **Staying compatible.** Own write paths (stage 3) must never produce a repository upstream Git rejects. Differential testing is the gate; any doubt keeps the operation on the runner.
7. **Performance of the runner.** Sandbox set-up per Git call may exceed the 40 ms budget; a long-lived runner process per repository operation may be needed.
8. **Case-insensitive or normalising file systems** on removable media change the path-safety rules; ZFS datasets here are case-sensitive by default, others need a check at open.
9. **Unverified.** Whether the Git package builds and passes its tests on HardenedBSD 15-STABLE with the hardening features on; whether reftable is needed at first release.
