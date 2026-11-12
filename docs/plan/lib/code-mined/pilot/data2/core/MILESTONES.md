## 28% (7000 of 24825) at 2026-11-13 00:15:10

## core run

Functions: 7000

### Label by kind

| **all** | 7000 | 690 | 960 | 865 | 595 | 562 | 3287 | 0 | **1650 (23.6%)** |
| allocator | 214 | 7 | 13 | 50 | 17 | 41 | 84 | 0 | 20 (9%) |
| arith-helper | 2966 | 427 | 463 | 211 | 295 | 133 | 1426 | 0 | 890 (30%) |
| bit-op | 1 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 (0%) |
| bitmap | 201 | 22 | 38 | 20 | 20 | 8 | 93 | 0 | 60 (30%) |
| bitmask | 21 | 7 | 6 | 0 | 3 | 1 | 4 | 0 | 13 (62%) |
| error-path | 958 | 93 | 142 | 132 | 32 | 63 | 489 | 0 | 235 (25%) |
| init-teardown | 1556 | 100 | 216 | 263 | 194 | 152 | 621 | 0 | 316 (20%) |
| kind | n | PROVED | PROVED-ASSUMING | CONTRACT-ONLY | FALSE | FAILED | UNBUILT | BOUNDED | proved (P+PA) |
| list-op | 79 | 0 | 0 | 19 | 0 | 21 | 38 | 0 | 0 (0%) |
| lock-or-sync | 205 | 5 | 22 | 30 | 7 | 35 | 106 | 0 | 27 (13%) |
| parser | 481 | 16 | 36 | 73 | 10 | 68 | 271 | 0 | 52 (11%) |
| refcount | 113 | 1 | 12 | 16 | 6 | 10 | 67 | 0 | 13 (12%) |
| ring-buffer | 205 | 12 | 12 | 51 | 11 | 30 | 87 | 0 | 24 (12%) |
|---|---|---|---|---|---|---|---|---|---|

### Label by tree

- other: 20 functions, PROVED 0, PROVED-ASSUMING 0, CONTRACT-ONLY 0, FALSE 0, FAILED 0, UNBUILT 20
- linux: 4432 functions, PROVED 615, PROVED-ASSUMING 828, CONTRACT-ONLY 742, FALSE 514, FAILED 456, UNBUILT 1241
- bsd-kernel: 826 functions, PROVED 2, PROVED-ASSUMING 99, CONTRACT-ONLY 89, FALSE 40, FAILED 54, UNBUILT 538
- linux-tools: 277 functions, PROVED 32, PROVED-ASSUMING 16, CONTRACT-ONLY 7, FALSE 12, FAILED 19, UNBUILT 190
- bsd-user: 1445 functions, PROVED 41, PROVED-ASSUMING 17, CONTRACT-ONLY 27, FALSE 29, FAILED 33, UNBUILT 1298

### Why not built (UNBUILT causes, normalised)

-     252 C++ source
-     118 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/contrib/llvm-project
-     109 [kernel:annot-error] unit.c: Warning: unbound logic variable true. Ignoring logic specific
-      83 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/sys/sys/vnode.h:629:
-      77 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/crypto/openssl/test/
-      54 [kernel] unit.c: User Error: bitfield width is not a valid integer constant [kernel] Frama
-      48 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/crypto/openssl/crypt
-      41 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/crypto/openssh/inclu
-      33  syntax error: before or at token: ;
-      28 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/contrib/libarchive/l
-      25 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/contrib/processor-tr
-      20 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/sys/contrib/dev/athk
-      19 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/sys/contrib/openzfs/
-      18 [kernel:annot-error] unit.c: Warning: unexpected token '{'

### Per-unit assumptions on proved functions (count)

- bv-sat-uncertified: 243
- callees-without-spec: 303
- config: 164
- header-function-in-probe-unit: 250
- pointer-or-memory-precondition: 195
- pointers-modelled-as-disjoint-malloc-objects(no-aliasing,sizes-from-valid): 23
- probe-adds-module.h-kernel.h-slab.h-device.h: 250
- stub-headers-EMPTY(option-headers-assumed-off): 97
- unwind=10: 32
- unwind=34: 15
- unwind=66: 1

## 28% (7000 of 24825) at 2026-11-13 01:25:06

## core run

Functions: 7377

### Label by kind

| **all** | 7377 | 736 | 1011 | 913 | 627 | 588 | 3461 | 0 | **1747 (23.7%)** |
| allocator | 221 | 8 | 13 | 51 | 17 | 42 | 88 | 0 | 21 (10%) |
| arith-helper | 3129 | 454 | 491 | 224 | 313 | 135 | 1501 | 0 | 945 (30%) |
| bit-op | 1 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 (0%) |
| bitmap | 212 | 22 | 40 | 20 | 23 | 9 | 98 | 0 | 62 (29%) |
| bitmask | 22 | 7 | 7 | 0 | 3 | 1 | 4 | 0 | 14 (64%) |
| error-path | 1011 | 99 | 148 | 141 | 34 | 66 | 516 | 0 | 247 (24%) |
| init-teardown | 1634 | 112 | 226 | 275 | 202 | 160 | 649 | 0 | 338 (21%) |
| kind | n | PROVED | PROVED-ASSUMING | CONTRACT-ONLY | FALSE | FAILED | UNBUILT | BOUNDED | proved (P+PA) |
| list-op | 87 | 0 | 0 | 21 | 0 | 24 | 41 | 0 | 0 (0%) |
| lock-or-sync | 216 | 5 | 22 | 32 | 7 | 39 | 111 | 0 | 27 (12%) |
| parser | 513 | 16 | 40 | 79 | 10 | 72 | 289 | 0 | 56 (11%) |
| refcount | 118 | 1 | 12 | 17 | 7 | 10 | 70 | 0 | 13 (11%) |
| ring-buffer | 213 | 12 | 12 | 53 | 11 | 30 | 93 | 0 | 24 (11%) |
|---|---|---|---|---|---|---|---|---|---|

### Label by tree

- other: 20 functions, PROVED 0, PROVED-ASSUMING 0, CONTRACT-ONLY 0, FALSE 0, FAILED 0, UNBUILT 20
- linux: 4686 functions, PROVED 657, PROVED-ASSUMING 874, CONTRACT-ONLY 785, FALSE 543, FAILED 477, UNBUILT 1314
- bsd-kernel: 866 functions, PROVED 2, PROVED-ASSUMING 103, CONTRACT-ONLY 90, FALSE 40, FAILED 56, UNBUILT 571
- linux-tools: 292 functions, PROVED 34, PROVED-ASSUMING 17, CONTRACT-ONLY 7, FALSE 12, FAILED 20, UNBUILT 201
- bsd-user: 1513 functions, PROVED 43, PROVED-ASSUMING 17, CONTRACT-ONLY 31, FALSE 32, FAILED 35, UNBUILT 1355

### Why not built (UNBUILT causes, normalised)

-     259 C++ source
-     124 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/contrib/llvm-project
-     117 [kernel:annot-error] unit.c: Warning: unbound logic variable true. Ignoring logic specific
-      87 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/sys/sys/vnode.h:629:
-      80 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/crypto/openssl/test/
-      58 [kernel] unit.c: User Error: bitfield width is not a valid integer constant [kernel] Frama
-      50 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/crypto/openssl/crypt
-      43 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/crypto/openssh/inclu
-      33  syntax error: before or at token: ;
-      28 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/contrib/libarchive/l
-      26 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/contrib/processor-tr
-      20 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/sys/contrib/openzfs/
-      20 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/sys/contrib/dev/athk
-      19 preprocess failed: /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/hbsd/src/sys/contrib/dev/acpi

### Per-unit assumptions on proved functions (count)

- bv-sat-uncertified: 257
- callees-without-spec: 319
- config: 170
- header-function-in-probe-unit: 268
- pointer-or-memory-precondition: 206
- pointers-modelled-as-disjoint-malloc-objects(no-aliasing,sizes-from-valid): 24
- probe-adds-module.h-kernel.h-slab.h-device.h: 268
- stub-headers-EMPTY(option-headers-assumed-off): 101
- unwind=10: 33
- unwind=130: 1
- unwind=34: 16
- unwind=66: 1

