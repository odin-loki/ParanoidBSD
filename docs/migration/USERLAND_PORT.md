# Userland C++23 Port Order (Wave 2)

Progressive conversion of `hbsd/src/lib`, `bin`, `sbin`, and `usr.*` into
Capsicum-first C++23 modules under `pbsd/userland/`. Consume
`docs/migration/c_inventory.csv` file-by-file; do not edit plan files.

## Module map

| HBSD source | PBSD module | Status |
|-------------|-------------|--------|
| `lib/libcapsicum/capsicum_helpers.h` | `pbsd.userland.capsicum` + `pbsd.userland.capsicum.helpers` | Scaffold |
| `lib/libcasper/` | `pbsd.userland.casper` | Scaffold |
| `lib/libjail/` | `pbsd.userland.jail` | Scaffold |
| `lib/libutil/` | `pbsd.userland.util` | Scaffold |

All userland modules link `pbsd_handles` and compile with `-fno-exceptions -fno-rtti`
to match nucleus BMI flags.

## Progressive port order

Port in dependency order so downstream tools can link incrementally:

1. **libcapsicum helpers** — `pbsd.userland.capsicum.helpers` replaces inline
   `caph_*` helpers from `capsicum_helpers.h`. Rights narrowing uses
   `UniqueHandle<FdObject>` instead of raw `cap_rights_limit(2)` on ambient fds.
2. **libcapsicum core** — syscall surface (`cap_enter`, `cap_rights_limit`) moves
   into `pbsd.userland.capsicum` with `Sandbox::enter()` gating ambient authority.
3. **libcasper** — `pbsd.userland.casper` models `cap_channel_t` as
   `ChannelHandle`; service proxies (cap_dns, cap_pwd, …) follow as partitions.
4. **libjail** — `pbsd.userland.jail` wraps `jailparam`/`jail_setv` with lineage
   tracking so jail creation is auditable.
5. **libutil** — `pbsd.userland.util` ports pidfile, property, humanize_number,
   flopen, and login_* helpers used by sbin/bin.
6. **bin/sbin/usr.bin** — re-link against `pbsd_userland` INTERFACE target; delete
   corresponding `.c` rows from inventory as each utility lands.

## Capsicum-first API rules

- No raw fd survives a public module boundary: adopt via `FdHandle`, narrow before I/O.
- `cap_enter` equivalents require explicit `Sandbox::enter()`; callers must drop
  ambient rights first.
- Casper channels are opened with explicit `CapabilityRights`; child services receive
  grants via `LineageTree`, not inherited ambient nvlist handles.
- Jail parameters are set through `JailHandle` with write-right checks.

## Bridge to HBSD

`pbsd/userland/capsicum/pbsd.userland.capsicum.helpers.cppm` is the thin C++23
bridge that will replace `hbsd/src/lib/libcapsicum/capsicum_helpers.h` over time.
During transition, hosted tests may dual-link the legacy header until Wave 9 purge.

## Build

```bash
cmake -S pbsd -B build -G Ninja -DCMAKE_CXX_COMPILER=clang++
cmake --build build --target pbsd_userland
```

## Remaining for full port

- Implement real syscall shims (`cap_rights_limit`, `cap_enter`, `jail_setv`).
- Port libcasper services (`cap_dns`, `cap_net`, `cap_pwd`, …) as module partitions.
- Convert each `hbsd/src/lib/libutil/*.c` unit with SI harness coverage.
- Re-link `hbsd/src/bin`, `sbin`, `usr.bin` against `pbsd_userland`.
- Remove `capsicum_helpers.h` from HBSD once all callers import modules.
