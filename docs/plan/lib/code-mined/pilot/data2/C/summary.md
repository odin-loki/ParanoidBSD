## sample C

Functions: 200

### Label by kind

| **all** | 200 | 17 | 30 | 16 | 13 | 41 | 80 | **47 (23.5%)** |
| allocator | 15 | 1 | 1 | 4 | 1 | 4 | 2 | 2 (13%) |
| arith-helper | 50 | 11 | 3 | 2 | 4 | 4 | 26 | 14 (28%) |
| bitmap | 15 | 3 | 5 | 0 | 1 | 0 | 6 | 8 (53%) |
| bitmask | 8 | 0 | 7 | 0 | 0 | 1 | 0 | 7 (88%) |
| error-path | 20 | 1 | 4 | 3 | 2 | 2 | 8 | 5 (25%) |
| init-teardown | 20 | 0 | 5 | 0 | 3 | 5 | 7 | 5 (25%) |
| kind | n | PROVED | PROVED-ASSUMING | CONTRACT-ONLY | FALSE | FAILED | UNBUILT | proved (P+PA) |
| list-op | 10 | 0 | 0 | 2 | 0 | 2 | 6 | 0 (0%) |
| lock-or-sync | 15 | 0 | 1 | 0 | 1 | 6 | 7 | 1 (7%) |
| parser | 20 | 1 | 1 | 2 | 1 | 7 | 8 | 2 (10%) |
| refcount | 12 | 0 | 2 | 2 | 0 | 5 | 3 | 2 (17%) |
| ring-buffer | 15 | 0 | 1 | 1 | 0 | 5 | 7 | 1 (7%) |
|---|---|---|---|---|---|---|---|---|

### Label by tree

- linux: 133 functions, PROVED 16, PROVED-ASSUMING 24, CONTRACT-ONLY 14, FALSE 10, FAILED 33, UNBUILT 33
- bsd-kernel: 25 functions, PROVED 0, PROVED-ASSUMING 6, CONTRACT-ONLY 1, FALSE 1, FAILED 5, UNBUILT 12
- linux-tools: 6 functions, PROVED 1, PROVED-ASSUMING 0, CONTRACT-ONLY 0, FALSE 2, FAILED 1, UNBUILT 2
- bsd-user: 36 functions, PROVED 0, PROVED-ASSUMING 0, CONTRACT-ONLY 1, FALSE 0, FAILED 2, UNBUILT 33

### Why not built (UNBUILT causes, normalised)

-       6 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/crypto/openssl/test/
-       5 [kernel] unit.c: User Error: bitfield width is not a valid integer constant [kernel] Frama
-       4 C++ source
-       3 [kernel:annot-error] unit.c: Warning: unbound logic variable true. Ignoring logic specific
-       2 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/sys/contrib/libsodiu
-       1 preprocess failed: kbuild produced no flags for drivers/s390/block/scm_blk.c (see /home/od
-       1 preprocess failed: kbuild produced no flags for drivers/net/ethernet/ibm/ehea/ehea_main.c 
-       1 preprocess failed: kbuild produced no flags for drivers/gpu/drm/amd/display/dmub/src/dmub_
-       1 preprocess failed: kbuild produced no flags for drivers/gpu/drm/amd/display/dc/basics/vect
-       1 preprocess failed: kbuild produced no flags for arch/mips/rb532/time.c (see /home/odin/kb/
-       1 preprocess failed: kbuild produced no flags for arch/arm64/kvm/hyp/nvhe/mem_protect.c (see
-       1 preprocess failed: kbuild produced no flags for arch/arm64/kvm/emulate-nested.c (see /home
-       1 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/usr.sbin/bluetooth/s
-       1 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/usr.sbin/bhyve/bhyve

### Per-unit assumptions on proved functions (count)

- bv-sat-uncertified: 10
- callees-without-spec: 6
- config: 6
- header-function-in-probe-unit: 8
- pointer-or-memory-precondition: 5
- probe-adds-module.h-kernel.h-slab.h-device.h: 8
- stub-headers-EMPTY(option-headers-assumed-off): 6
