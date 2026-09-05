# The vendor tree, and how to sync it

`hbsd/src` is HardenedBSD. It arrived as **one commit with no remote, no
submodule and no vendor branch** — a flat copy, upstream history and file
modes discarded.

Everything that has gone wrong with the tree traces to that single decision:

| Symptom | Cause |
|---|---|
| `contrib/bmake/configure: Permission denied`, an hour into a build | 7,970 scripts imported as mode 100644 |
| `don't know how to make sys/dev/hpt27xx/amd64-elf.hpt27xx_lib.o` | `git add` ran ignore rules over files upstream tracks; 114 lost |
| `usr.bin/id` gone while `usr.bin/Makefile:57` still lists it | `ID` in the vendor `.gitignore` matching a directory on a case-insensitive filesystem |
| `realpath: sys/i386/include: No such file` | whole architectures dropped, but `kmod.mk` still symlinks `i386` for amd64 |
| "what has PBSD actually changed?" unanswerable | no base to diff against |

Each was found and fixed one at a time, at the cost of a build run each. A
merge would have produced none of them.

## The vendor branch

`vendor/hardenedbsd` holds the vendor tree alone, at `hbsd/src/…`, with no
PBSD files. `main` carries a `-s ours` merge of it, which changed no file
and exists only to establish a merge base.

The base is **this tree, not a fetched upstream revision**. The snapshot's
upstream revision is not recoverable: the reference clone is shallow and
HardenedBSD does not tag what was taken. That is sound here because PBSD has
barely touched the vendor tree. Of the 11,341 files that differ from
`hardened/15-stable/main`, exactly **four** carry a PBSD change:

| File | Change |
|---|---|
| `Makefile.inc1` | lib32 and Tier-2 compat architectures removed |
| `share/mk/src.opts.mk` | no 32-bit ARM LLVM target |
| `sys/conf/kern.mk` | freestanding kernel C++23 flags |
| `sys/conf/kmod.mk` | C++23 module TUs, dual-link pattern |

About 100 lines in total. The other 11,337 are upstream moving on since the
snapshot — 6,846 of them `contrib/llvm-project` alone, which is a whole LLVM
version, not anyone's edit.

Those four files are the one place the base is wrong, and the failure is
silent: git sees base == ours, takes upstream's side, and reports no
conflict. Nothing in git can catch that, so it is checked instead:

```sh
python3 tools/check_pbsd_marks.py
```

## Syncing

```sh
# 1. Get upstream.
git clone --depth 1 -b hardened/15-stable/main \
    https://github.com/HardenedBSD/hardenedbsd /tmp/hbsd-new

# 2. Replace the vendor branch's tree wholesale. Modes come with it.
git checkout vendor/hardenedbsd
rm -rf hbsd/src
mkdir -p hbsd/src
(cd /tmp/hbsd-new && git archive HEAD) | tar -x -C hbsd/src
git add -A hbsd/src
git commit -m "Vendor import: HardenedBSD <rev>, <date>"

# 3. Merge into main. Conflicts here are real and worth reading.
git checkout main
git merge vendor/hardenedbsd

# 4. Check what the merge cannot tell you about.
python3 tools/check_pbsd_marks.py
python3 tools/check_vendor_manifest.py
python3 tools/check_subdirs.py hbsd/src --fail-on-missing
python3 tools/check_exec_bits.py
```

Step 2 uses `git archive | tar`, not `cp -r`, because that is what carries
the executable bits. Step 4 is not optional: three of the four checks exist
because the thing they check for has already happened once.

Once a sync has been done this way, the vendor branch's base is a real
upstream revision and the caveat about the four files stops applying — the
merge will conflict on them properly, like it should have all along.
