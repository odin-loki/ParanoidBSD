# PBSD Glossary

| Term | Definition |
|---|---|
| **PBSD** | The OS project name. The kernel and overall system. |
| **BIFROST** | The VM/hypervisor subsystem and Windows interop bridge (Phase 6). |
| **UDA** | Universal Driver Architecture — descriptor-engine split for hardware support. |
| **Descriptor** | A declarative data structure describing one chip's register layout and init sequence. Never contains executable logic beyond the bytecode interpreter. |
| **Generic Engine** | The UDA component that interprets a descriptor and drives a device class (block, net, display, input). |
| **UniqueHandle<T>** | Move-only, single-owner kernel resource handle. The default for almost all kernel objects. |
| **BorrowedHandle<T>** | Non-owning view of a handle. Cannot outlive the owning handle. Annotated `[[kernel::no_escape]]`. |
| **SharedHandle<T>** | Refcounted handle. Rare; requires individual audit justification. |
| **CapabilityRights** | Bitmask attached to every handle. Rights only narrow on grant/duplication. |
| **Revocation tree** | Per-process capability lineage. Revoking one handle invalidates all derived handles. |
| **Profile** | A build configuration: embedded, server, or workstation. Same kernel source; different modules compiled in. |
| **PROVENANCE.md** | Living log mapping every module to its spec source (standard/datasheet/original). The legal clean-room trail. |
| **SPEC.md** | Per-module contract document. Must exist before implementation starts. |
| **Phase N** | Project plan phase number. See master-plan.md. |
