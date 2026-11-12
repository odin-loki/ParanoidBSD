## sample A

Functions: 200

### Label by kind

| **all** | 200 | 15 | 20 | 30 | 10 | 27 | 96 | **35 (17.5%)** |
| allocator | 15 | 0 | 1 | 3 | 1 | 3 | 7 | 1 (7%) |
| arith-helper | 50 | 5 | 8 | 3 | 2 | 6 | 26 | 13 (26%) |
| bitmap | 15 | 0 | 2 | 2 | 1 | 1 | 9 | 2 (13%) |
| bitmask | 8 | 4 | 1 | 0 | 1 | 1 | 1 | 5 (62%) |
| error-path | 20 | 1 | 5 | 2 | 2 | 3 | 7 | 6 (30%) |
| init-teardown | 20 | 1 | 1 | 4 | 3 | 4 | 6 | 2 (10%) |
| kind | n | PROVED | PROVED-ASSUMING | CONTRACT-ONLY | FALSE | FAILED | UNBUILT | proved (P+PA) |
| list-op | 10 | 0 | 0 | 1 | 0 | 2 | 7 | 0 (0%) |
| lock-or-sync | 15 | 1 | 1 | 1 | 0 | 0 | 12 | 2 (13%) |
| parser | 20 | 2 | 0 | 6 | 0 | 3 | 9 | 2 (10%) |
| refcount | 12 | 0 | 0 | 5 | 0 | 3 | 3 | 0 (0%) |
| ring-buffer | 15 | 1 | 1 | 3 | 0 | 1 | 9 | 2 (13%) |
|---|---|---|---|---|---|---|---|---|

### Label by tree

- linux: 119 functions, PROVED 13, PROVED-ASSUMING 17, CONTRACT-ONLY 29, FALSE 8, FAILED 26, UNBUILT 25
- bsd-kernel: 33 functions, PROVED 1, PROVED-ASSUMING 2, CONTRACT-ONLY 0, FALSE 2, FAILED 0, UNBUILT 27
- linux-tools: 7 functions, PROVED 0, PROVED-ASSUMING 1, CONTRACT-ONLY 1, FALSE 0, FAILED 1, UNBUILT 4
- bsd-user: 41 functions, PROVED 1, PROVED-ASSUMING 0, CONTRACT-ONLY 0, FALSE 0, FAILED 0, UNBUILT 40

### Why not built (UNBUILT causes, normalised)

-      13 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/sys/sys/bus.h:893:10
-       4 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/contrib/sqlite3/sqli
-       3 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/sys/contrib/openzfs/
-       3 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/crypto/openssl/crypt
-       3 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/contrib/llvm-project
-       3 C++ source
-       2 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/crypto/krb5/src/lib/
-       2 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/contrib/wpa/src/util
-       2 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/contrib/processor-tr
-       2  syntax error: before or at token: ;
-       1 preprocess failed: kbuild produced no flags for drivers/tty/mips_ejtag_fdc.c (see /home/od
-       1 preprocess failed: kbuild produced no flags for drivers/tty/amiserial.c (see /home/odin/kb
-       1 preprocess failed: kbuild produced no flags for drivers/gpu/drm/msm/dsi/phy/dsi_phy_10nm.c
-       1 preprocess failed: kbuild produced no flags for drivers/gpu/drm/amd/pm/swsmu/smu14/smu_v14

### Per-unit assumptions on proved functions (count)

- bv-sat-uncertified: 2
- callees-without-spec: 7
- config: 3
- header-function-in-probe-unit: 7
- pointer-or-memory-precondition: 4
- probe-adds-module.h-kernel.h-slab.h-device.h: 7
- stub-headers: 2
