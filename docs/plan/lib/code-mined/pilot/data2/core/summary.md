## core run

Functions: 8000

### Label by kind

| **all** | 8000 | 788 | 1090 | 983 | 678 | 635 | 3784 | 0 | **1878 (23.5%)** |
| allocator | 241 | 8 | 13 | 57 | 19 | 45 | 97 | 0 | 21 (9%) |
| arith-helper | 3431 | 490 | 537 | 248 | 344 | 152 | 1649 | 0 | 1027 (30%) |
| bit-op | 1 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 (0%) |
| bitmap | 228 | 24 | 43 | 21 | 25 | 10 | 105 | 0 | 67 (29%) |
| bitmask | 23 | 7 | 8 | 0 | 3 | 1 | 4 | 0 | 15 (65%) |
| error-path | 1081 | 103 | 158 | 152 | 36 | 69 | 556 | 0 | 261 (24%) |
| init-teardown | 1758 | 121 | 239 | 293 | 213 | 173 | 709 | 0 | 360 (20%) |
| kind | n | PROVED | PROVED-ASSUMING | CONTRACT-ONLY | FALSE | FAILED | UNBUILT | BOUNDED | proved (P+PA) |
| list-op | 92 | 0 | 1 | 22 | 0 | 24 | 44 | 0 | 1 (1%) |
| lock-or-sync | 235 | 5 | 23 | 33 | 8 | 41 | 125 | 0 | 28 (12%) |
| parser | 551 | 17 | 43 | 82 | 11 | 76 | 315 | 0 | 60 (11%) |
| refcount | 125 | 1 | 12 | 17 | 7 | 10 | 76 | 0 | 13 (10%) |
| ring-buffer | 234 | 12 | 13 | 58 | 12 | 34 | 103 | 0 | 25 (11%) |
|---|---|---|---|---|---|---|---|---|---|

### Label by tree

- other: 23 functions, PROVED 0, PROVED-ASSUMING 0, CONTRACT-ONLY 0, FALSE 0, FAILED 0, UNBUILT 23
- linux: 5067 functions, PROVED 705, PROVED-ASSUMING 937, CONTRACT-ONLY 847, FALSE 586, FAILED 519, UNBUILT 1436
- bsd-kernel: 946 functions, PROVED 2, PROVED-ASSUMING 118, CONTRACT-ONLY 94, FALSE 43, FAILED 59, UNBUILT 626
- linux-tools: 314 functions, PROVED 35, PROVED-ASSUMING 17, CONTRACT-ONLY 7, FALSE 14, FAILED 21, UNBUILT 219
- bsd-user: 1650 functions, PROVED 46, PROVED-ASSUMING 18, CONTRACT-ONLY 35, FALSE 35, FAILED 36, UNBUILT 1480

### Why not built (UNBUILT causes, normalised)

-     283 C++ source
-     140 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/contrib/llvm-project
-     120 [kernel:annot-error] unit.c: Warning: unbound logic variable true. Ignoring logic specific
-      93 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/sys/sys/vnode.h:629:
-      87 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/crypto/openssl/test/
-      68 [kernel] unit.c: User Error: bitfield width is not a valid integer constant [kernel] Frama
-      52 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/crypto/openssl/crypt
-      48 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/crypto/openssh/inclu
-      36  syntax error: before or at token: ;
-      29 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/contrib/processor-tr
-      29 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/contrib/libarchive/l
-      23 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/sys/contrib/openzfs/
-      22 [kernel:annot-error] unit.c: Warning: unexpected token '{'
-      21 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/sys/contrib/dev/acpi

### Per-unit assumptions on proved functions (count)

- bv-sat-uncertified: 278
- callees-without-spec: 337
- config: 182
- header-function-in-probe-unit: 289
- pointer-or-memory-precondition: 222
- pointers-modelled-as-disjoint-malloc-objects(no-aliasing,sizes-from-valid): 26
- probe-adds-module.h-kernel.h-slab.h-device.h: 289
- stub-headers-EMPTY(option-headers-assumed-off): 115
- unwind=10: 36
- unwind=130: 1
- unwind=34: 16
- unwind=66: 1
