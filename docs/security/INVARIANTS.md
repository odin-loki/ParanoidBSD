# PBSD Security Invariants

These properties MUST hold across every contract and every module.  
They are stated once here and referenced by each contract's SPEC.md.  
If an implementation would violate any invariant, the implementation is wrong, not the invariant.

## SI-1: Rights only narrow
No operation grants a capability with rights exceeding those held by the caller.
`child_rights ⊆ caller_rights` always.

## SI-2: No ambient authority
A process can only act on resources for which it holds an explicit handle.
There is no "root can do anything" escape hatch in the native capability model.

## SI-3: W^X enforced everywhere
No memory page is simultaneously writable and executable at any point in time.

## SI-4: All kernel memory zero-initialised before use
No kernel allocation may be returned to a caller containing data from a prior use.

## SI-5: No blocking in interrupt context
Functions tagged as ISR entry points must not call any function that can sleep, block, or acquire a sleeping lock. Enforced by the static analyser Layer 4.

## SI-6: Revocation is total and immediate
Revoking a handle invalidates it and all handles derived from it in the same revocation tree, atomically from the holder's perspective.

## SI-7: Descriptor cannot self-escalate
A UDA descriptor executes only within the MMIO/IRQ capability scope granted to its generic engine. A descriptor cannot reference memory or I/O outside that grant.

## SI-8: VM guest is capability-isolated
The BIFROST VM guest (Windows) is a kernel object like any other. Its MMIO access, memory, and I/O are bounded by the VmHandle capability granted to it. The guest cannot access host kernel memory.
