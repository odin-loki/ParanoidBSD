## sample A

Functions: 200

### Label by kind

| **all** | 200 | 17 | 22 | 42 | 15 | 15 | 86 | 0 | **39 (19.5%)** |
| allocator | 15 | 1 | 1 | 5 | 2 | 1 | 5 | 0 | 2 (13%) |
| arith-helper | 50 | 6 | 8 | 7 | 4 | 1 | 24 | 0 | 14 (28%) |
| bitmap | 15 | 0 | 3 | 2 | 2 | 0 | 8 | 0 | 3 (20%) |
| bitmask | 8 | 4 | 1 | 0 | 1 | 1 | 1 | 0 | 5 (62%) |
| error-path | 20 | 1 | 5 | 3 | 2 | 2 | 7 | 0 | 6 (30%) |
| init-teardown | 20 | 1 | 1 | 7 | 3 | 1 | 6 | 0 | 2 (10%) |
| kind | n | PROVED | PROVED-ASSUMING | CONTRACT-ONLY | FALSE | FAILED | UNBUILT | BOUNDED | proved (P+PA) |
| list-op | 10 | 0 | 0 | 2 | 0 | 2 | 6 | 0 | 0 (0%) |
| lock-or-sync | 15 | 1 | 1 | 1 | 0 | 1 | 11 | 0 | 2 (13%) |
| parser | 20 | 2 | 1 | 5 | 1 | 3 | 7 | 0 | 3 (15%) |
| refcount | 12 | 0 | 0 | 7 | 0 | 1 | 3 | 0 | 0 (0%) |
| ring-buffer | 15 | 1 | 1 | 3 | 0 | 2 | 8 | 0 | 2 (13%) |
|---|---|---|---|---|---|---|---|---|---|

### Label by tree

- linux: 119 functions, PROVED 13, PROVED-ASSUMING 17, CONTRACT-ONLY 39, FALSE 12, FAILED 12, UNBUILT 25
- bsd-kernel: 33 functions, PROVED 1, PROVED-ASSUMING 3, CONTRACT-ONLY 0, FALSE 3, FAILED 2, UNBUILT 23
- linux-tools: 7 functions, PROVED 0, PROVED-ASSUMING 2, CONTRACT-ONLY 1, FALSE 0, FAILED 0, UNBUILT 4
- bsd-user: 41 functions, PROVED 3, PROVED-ASSUMING 0, CONTRACT-ONLY 2, FALSE 0, FAILED 1, UNBUILT 34

### Why not built (UNBUILT causes, normalised)

-       3 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/crypto/openssl/crypt
-       3 [kernel:annot-error] unit.c: Warning: unbound logic variable true. Ignoring logic specific
-       3 C++ source
-       2 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/crypto/krb5/src/lib/
-       2 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/contrib/processor-tr
-       2 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/contrib/llvm-project
-       2  syntax error: before or at token: ;
-       1 preprocess failed: kbuild produced no flags for drivers/tty/mips_ejtag_fdc.c (see /home/od
-       1 preprocess failed: kbuild produced no flags for drivers/tty/amiserial.c (see /home/odin/kb
-       1 preprocess failed: kbuild produced no flags for drivers/gpu/drm/msm/dsi/phy/dsi_phy_10nm.c
-       1 preprocess failed: kbuild produced no flags for drivers/gpu/drm/amd/pm/swsmu/smu14/smu_v14
-       1 preprocess failed: kbuild produced no flags for drivers/gpu/drm/amd/pm/powerplay/hwmgr/pro
-       1 preprocess failed: kbuild produced no flags for arch/s390/crypto/aes_s390.c (see /home/odi
-       1 preprocess failed: kbuild produced no flags for arch/powerpc/mm/book3s64/slb.c (see /home/

### Per-unit assumptions on proved functions (count)

- bv-sat-uncertified: 3
- callees-without-spec: 8
- config: 3
- header-function-in-probe-unit: 7
- pointer-or-memory-precondition: 5
- pointers-modelled-as-disjoint-malloc-objects(no-aliasing,sizes-from-valid): 1
- probe-adds-module.h-kernel.h-slab.h-device.h: 7
- stub-headers-EMPTY(option-headers-assumed-off): 1
- stub-headers: 2
