# SPEC: PBSD Handle & Capability Closed Set

**Status:** Frozen for Wave 1  
**Invariants:** SI-1, SI-2, SI-6, SI-8  
**Module:** `pbsd.handles`, `pbsd.rights`, `pbsd.cap.lineage`  
**Port context:** ParanoidBSD full C++23 port — handles replace raw pointers across kernel and userland PBSD surfaces.

## Closed type set

| Type | Ownership | Copy | Notes |
|------|-----------|------|-------|
| `UniqueHandle<T>` | Exclusive | Move-only | Default for almost all kernel objects |
| `BorrowedHandle<T>` | Non-owning | Copyable | Must not outlive lender; `no_escape` attribute |
| `SharedHandle<T>` | Refcounted | Copyable | Rare; requires audit justification in PROVENANCE |
| `VmHandle` | Exclusive VM object | Move-only | SI-8; BIFROST guest slot |

## Rights model (SI-1, SI-2)

`CapabilityRights` is a closed bitmask. Every handle carries `(rights, LineageId)`.

| Operation | Produces | Rule |
|-----------|----------|------|
| `grant(subset, tree)` | `BorrowedHandle` or error | `subset ⊆ parent.rights` |
| `duplicate()` | `UniqueHandle` or error | Same rights; new lineage child |
| `narrow(subset)` | `BorrowedHandle` | Strict subset only |
| `borrow(subset)` | `BorrowedHandle` | Temporary view; lender must stay alive |

Native APIs take handles — no ambient authority (SI-2). Callers cannot synthesise rights from raw pointers.

## Lineage (SI-6)

Every handle stores `LineageId`. `grant` / `borrow` / `duplicate` allocate child ids in the tree. `revoke(id)` invalidates the node and all descendants atomically from the holder's perspective.

## KernelObject concept

```cpp
template<typename T>
concept KernelObject = requires(T* p) {
    { T::release(p) } noexcept;
};
```

`UniqueHandle<T>` calls `T::release` on destruction or move-assignment. No virtual destructor requirement — static release only.

## Forbidden patterns

1. No public API returns a raw `T*` that can be re-wrapped as a handle (`peek` / friend kernel ops only).
2. No `SharedHandle` without PROVENANCE entry and Wave 4+ audit hook.
3. No handle construction from integer FD / cookie without explicit `from_trusted_*` bridge (Capsicum Wave 4).
4. No `std::shared_ptr` / `std::unique_ptr` in freestanding kernel TUs — use closed set only.

## Analyser coupling

L2 ownership plugin (`pbsd-ownership`) validates use-after-move, double-release, and rights widen on this closed set. See `docs/specs/ANALYSER.md`.

## Proof artifacts

- `pbsd/tests/si_harness.cpp` — SI-1 / SI-6 unit checks
- `pbsd/handles/pbsd.handles.cppm` — reference implementation
