# SPEC: Capsicum Lineage Extension

**Status:** Frozen for Wave 1 (API) / Wave 4 (kernel integration)  
**Invariant:** SI-6  
**Port context:** ParanoidBSD full C++23 port — lineage generalises Capsicum rights revocation to the handle closed set.

## Model

Each handle holds `LineageId`. Operations allocate child ids in a tree:

| Event | Lineage effect |
|-------|----------------|
| Root create | New root id |
| `grant` / `borrow` / `duplicate` | Child id linked to parent |
| `revoke(id)` | Subtree marked invalid atomically |

After revoke, any use of a handle in the subtree returns `Status::Revoked` (or EINVAL at the C bridge).

## API surface (`pbsd.cap.lineage`)

```cpp
class LineageTree {
public:
    LineageId create_root();
    LineageId create_child(LineageId parent);
    void revoke(LineageId id);
    bool valid(LineageId id) const;
};
```

Handles store the id immutably for their lifetime; re-grant produces a new handle with a new id.

## Capsicum bridge (Wave 4)

Wave 4 maps lineage onto FreeBSD `struct file` / Capsicum rights (`cap_rights_t`) via thin `extern "C"` hooks in `pbsd/kernel/capsicum_bridge.cppm`:

| PBSD | FreeBSD |
|------|---------|
| `LineageId` | Shadow field / side table keyed by `struct file *` |
| `CapabilityRights` | `cap_rights_t` subset mapping |
| `revoke(id)` | `cap_revoke` + invalidation of derived fds |

Policy stays in `pbsd.cap.lineage`; the bridge performs mechanical translation only.

## Threat notes

Revoke races are a priority surface — see `docs/security/THREAT_MODEL.md`. Kernel bridge must hold the same locks as `filedesc` mutation.

## Proof artifacts

- `pbsd/lineage/pbsd.cap.lineage.cppm` — reference tree
- `pbsd/tests/si_harness.cpp` — revoke propagates to descendants
- `pbsd/kernel/capsicum_bridge.cppm` — integration scaffold (Wave 4)

## Related specs

- `docs/specs/HANDLES.md` — closed handle set
- `docs/specs/KERNEL_CXX_ABI.md` — dual-link C bridge pattern
