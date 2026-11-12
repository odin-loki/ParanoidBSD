<!-- SPDX-License-Identifier: AGPL-3.0-or-later -->
# Model Manager

By Odin Loch

| | |
|---|---|
| ID | `AI02` |
| Bundle | `org.pbsd.model-manager` |
| Category | ai |
| Priority | P1 |
| Backing repos | FORGE (`forge`, `forge-serve`, `.fpk`, inference, load balancer, RPC devices) |
| Status | Planned. Nothing built. |

## 1. Purpose

Model Manager is where a person brings models onto the machine, checks where they came from, decides which apps may use them, and sees how they run. It is the graphical front end for every `forge` and `forge-serve` command, and it administers `svc.model`, the one service through which all apps reach on-device models. It replaces command-line model handling and third-party model launchers.

## 2. Scope

### 2.1 What FORGE provides today (Verified)

| Fact | Source |
|---|---|
| `forge` commands: `info`, `import`, `inspect`, `run`, `bench`, `dump`, `keygen`, `sign`, `verify`, `encrypt`, `rpc-serve`, `security`, `script`. | `FORGE:docs/cli.md` |
| `forge import` reads a Hugging Face directory, a `.gguf` or an `.onnx` file and writes `.fpk`. 17 weight codecs plus presets `q4_k_m`, `q5_k_m`; options `--emb-codec`, `--template`, `--recipe forge\|llama`, `--output-tensor-type`, `--token-embedding-type`, `--tensor-type`, `--pure`, `--bf16-head`. GGUF tensors are copied verbatim; `--codec` is not applied to them. | `FORGE:docs/cli.md` (import), `FORGE:docs/modelfile.md` (Importers) |
| `.fpk` holds a manifest, a blueprint (graph text), a tokenizer, a tensor index and data; every section and tensor has a BLAKE3 hash; arbitrary named sections can be added (`set_section`). | `FORGE:docs/format.md`, `FORGE:docs/modelfile.md` |
| Signing is Ed25519 over a BLAKE3 digest of all sections except `signature`; several signers allowed; `key_id` is the first 4 bytes of BLAKE3(public key). With no trusted keys, any valid signature passes (integrity, not provenance). | `FORGE:docs/format.md` (Hashing and signing), `FORGE:docs/security.md` |
| Encryption seals only the `data` section with XChaCha20-Poly1305; metadata stays readable; keys come from `FORGE_MODEL_KEY` or `FORGE_MODEL_KEY_FILE`. | `FORGE:docs/format.md` (Encryption) |
| `forge keygen -o NAME` writes `NAME.key` (secret seed) and `NAME.pub` as files. | `FORGE:docs/cli.md` (keygen) |
| `forge info` prints CPU name and features, the SIMD table in use, one line per GPU runtime saying whether its library loads (not whether a device works), RAM, cgroup and `RLIMIT_AS` limits, and the host memory budget with its source. | `FORGE:docs/cli.md` (info), `FORGE:docs/memory.md` |
| `forge bench` measures prefill and decode throughput (`-p`, `-n`, `--warmup`, `--codec`); `--gemm N` times a matrix multiply per SIMD table; on CUDA/HIP it prints driver call counts. | `FORGE:docs/cli.md` (bench) |
| `forge-serve` serves one generative model: flags `--host` (default `127.0.0.1`), `--port` (8080), `--max-active`, `--ctx`, `--codec`, `--mem-limit`, `--max-queue`, `--kv-spill`, `--watch`, `--no-pin`, `--numa-node`, `--policy`. No authentication, no TLS, CORS `*`, IPv4 TCP only. | `FORGE:docs/serve.md:16-34`, `:62`, `:115` |
| `GET /metrics` gives scheduler counters (`forge_steps`, `forge_completed`, `forge_prefix_hits`, `forge_queue_waiting`, `forge_queue_active`, `forge_rejected_total`, `forge_kv_*`) and memory series (`forge_mem_*`). | `FORGE:docs/serve.md:90-91`, `FORGE:docs/memory.md` (Observability) |
| `forge-serve proxy` balances over backends with 7 strategies, per-backend circuit breakers, health checks, retries and backpressure; it has no TLS and buffers responses, so SSE arrives at the end. | `FORGE:docs/loadbalancer.md` |
| `forge rpc-serve` lends a local device to remote processes: tokens, Ed25519 keys, TLS, mTLS, client-certificate map, `allow=device\|scripts` permissions, remote scripts in sandboxed children. Without tokens or keys it is open to anyone who can reach it. | `FORGE:docs/cli.md` (rpc-serve), `FORGE:docs/rpc.md` |
| LoRA adapters load from an `.fpk` with `<weight>.lora_a` / `.lora_b` tensors and manifest `lora_alpha`. | `FORGE:docs/inference.md:25-26` |
| Strict loading: `FORGE_REQUIRE_SIGNED`, `FORGE_TRUSTED_KEYS`, and under a policy `integrity.models = required`. | `FORGE:docs/cli.md` (Strict loading, Security policy) |
| GPU status: CUDA ran on one real card; Metal on a CI runner; HIP, OpenCL, Level Zero on mock drivers only; Vulkan on software devices only. FreeBSD 14.3 runs the non-GPU tests in a VM. | `FORGE:README.md` (Status), `FORGE:docs/STATUS.md` |
| KV spill writes prompt-derived data to disk unencrypted. | `FORGE:docs/security.md` (Known gaps) |

### 2.2 Relation to `svc.model`

`svc.model` is planned as the daemon `forge-modeld` in FORGE-WP-29: a scheduler process plus one worker per loaded model, a model library on the verified dataset, `integrity.models = required` with a non-empty key list, and `ai.local` permissions with per-class scopes. Model Manager owns the service from the user's side: it is the only client of the service's administration interface, it defines the library record format in section 6.2 together with FORGE-WP-29.3, and every policy the service enforces is edited here. Apps never load models themselves.

### 2.3 In scope (all Proposed)

Model library; import, conversion and quantisation jobs; inspect and verify; sign and encrypt with keys held by SE04; trust policy; download with verification; hardware view; benchmarks; evaluation; per-app model permissions; memory and scheduling budgets; adapters; an optional serving dashboard for `forge-serve`, its proxy and `rpc-serve`; remote devices; disk usage; model cards.

### 2.4 Out of scope

| Item | Owner |
|---|---|
| Chatting with a model | AI01 |
| Key generation, storage, backup, hardware tokens | SE04 (Model Manager asks it to create and use keys) |
| The grant store and lineage tree for all capabilities | AD05 (Model Manager edits only `ai.local` scopes) |
| Firewall rules for a served endpoint | SE02 |
| Generic downloads and torrents | IN03 |
| Training and fine-tuning | none planned; adapters are imported, not trained here |
| Forecast bundles and Cypha classifier files | AI03 (they are registered in the library as class `plugin` only) |
| Package updates of FORGE itself | SY05 |
| System-wide process and GPU monitoring | SY04 |

## 3. Users and top tasks

Personas: **Noor**, who wants one good local model and never opens this app again; **Sam**, a developer who converts, quantises, benchmarks and serves models on his own LAN; **Ari**, an administrator who allows only models signed by two keys and audits which app used which model; **Kim**, a researcher comparing quantisations on a fixed evaluation set.

| # | Task | Target steps |
|---|---|---|
| T1 | Import a `.gguf` or Hugging Face folder and make it usable | 4 (Import, pick, accept defaults, Finish) |
| T2 | See whether a model is intact and who signed it | 1 (select it) |
| T3 | Sign a model with my key | 3 |
| T4 | Allow an app to use a model | 2 (Permissions, tick) |
| T5 | Revoke an app's access | 1 |
| T6 | Set the memory budget for models | 2 |
| T7 | Benchmark a model and compare with another | 3 |
| T8 | Re-quantise a model to a smaller codec | 4 |
| T9 | Download a model from a URL with a checksum | 5, one network prompt |
| T10 | See what is loaded now and unload it | 1–2 |
| T11 | Trust a new signer key | 3 |
| T12 | Start a local OpenAI-compatible endpoint | 4, one listen prompt |
| T13 | Free disk space (caches, duplicates) | 2 |
| T14 | Read a model's card and licence before first use | 1 |
| T15 | Add a remote compute device | 5 |

## 4. Features

Priority: M must, S should, C could. Phase: 1 usable core, 2 full feature, 3 advanced.

### 4.1 Library

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| AI02-F001 | Model list | `tableView`: name, class, architecture, parameters, codec, size, trust state, signer, licence, loaded, last used. Sortable, column chooser. | M | 1 |
| AI02-F002 | Grid view | `gridView` cards with class icon, size, trust badge. | C | 2 |
| AI02-F003 | Classes | Text generation, embeddings, speech-to-text, text-to-speech, OCR, vision, one-shot graph (ONNX imports), plugin (non-neural, e.g. Cypha), adapter. Derived from the manifest `session` and `architecture`. | M | 1 |
| AI02-F004 | Search and filter | By name, class, codec, signer, licence, trust state, tag, "used by app". | M | 1 |
| AI02-F005 | Tags and notes | Free tags and a private note per model. | S | 2 |
| AI02-F006 | Favourite and default | Mark a default per class; apps asking for "the default text model" get it. | M | 1 |
| AI02-F007 | Rename display name | The file's content hash stays the identity; the display name is library metadata. | S | 1 |
| AI02-F008 | Remove model | To the system trash (`svc.undo-trash`); refuses while loaded unless "Unload and remove". Lists the apps that lose access. | M | 1 |
| AI02-F009 | Duplicate detection | Same data hash under two names is shown once with aliases. | S | 2 |
| AI02-F010 | Family grouping | Quantisations of one source grouped under one row, expandable. | S | 2 |
| AI02-F011 | Library location | Models live on the verified dataset under the service's directory; the app shows the path and free space. A second location (external disk) can be added through the folder portal. | M | 1 |
| AI02-F012 | Offline models on removable media | A library location that is absent shows its models greyed with "media not present". | C | 3 |
| AI02-F013 | Recently used | Per model: last app, last time, total tokens (counts only, from the service). | S | 2 |
| AI02-F014 | Open with | `.fpk`, `.gguf`, `.onnx` opened from Files land in the import wizard or the inspect view; never loaded directly. | M | 1 |

### 4.2 Import, conversion, quantisation

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| AI02-F015 | Import wizard | Source: `.fpk` file, `.gguf` file, `.onnx` file, Hugging Face directory (`config.json`, `*.safetensors`, `tokenizer.json`). Picked through the portal. | M | 1 |
| AI02-F016 | Source check | Before conversion: detect format, architecture, size, tensor count; refuse unknown architectures with FORGE's own message. | M | 1 |
| AI02-F017 | Codec choice | `--codec`: `f32 f16 bf16 fp8_e4m3 fp8_e5m2 q8_0 q4_k q5_k q6_k q4_g32 q8_g32 mxfp8 mxfp6 mxfp4 mxint8 nvfp4 tq2`, presets `q4_k_m`, `q5_k_m`. Each with estimated size and a one-line note. | M | 1 |
| AI02-F018 | Recommended codec | Suggests a codec from the memory budget and the device ("fits in 12 GB with 8k context"). | S | 2 |
| AI02-F019 | Embedding codec | `--emb-codec`; default shown (`q8_0` when the main codec is quantised). | S | 1 |
| AI02-F020 | Template | `--template`: auto, or `llama`, `mixtral`, `deepseek_v2`, `qwen3`, `qwen3_5`. | S | 1 |
| AI02-F021 | Mixed-precision recipe | `--recipe forge` or `llama`; `--pure`; `--bf16-head`; `--output-tensor-type`; `--token-embedding-type`. Shown only for Hugging Face sources, as FORGE documents. | S | 2 |
| AI02-F022 | Per-tensor overrides | `--tensor-type REGEX=TYPE` rows in a small table with a live count of matching tensors. | C | 3 |
| AI02-F023 | GGUF note | For `.gguf` the codec controls are disabled with the reason: tensors are copied verbatim. | M | 1 |
| AI02-F024 | ONNX note | Lists the supported op set; an unsupported op is named before the job starts. | M | 2 |
| AI02-F025 | Import as a job | Runs in the job worker with progress, log, cancel; survives closing the window; result notification. | M | 1 |
| AI02-F026 | Sandboxed import | The importer parses untrusted files in a helper with only the source handle and an output handle. | M | 1 |
| AI02-F027 | Provenance record | Stored with the model: source kind, source file hashes (BLAKE3 and SHA-256), origin URL if downloaded, import options, FORGE version, date, user. | M | 1 |
| AI02-F028 | Re-quantise | New job from an existing f32/f16 `.fpk` or its original source to another codec. Result is a new library entry in the same family. | S | 2 |
| AI02-F029 | Batch conversion | Queue several codecs from one source. | C | 2 |
| AI02-F030 | Add card and licence | On import the wizard asks for a model card (Markdown) and a licence text or SPDX id; stored as `.fpk` sections `card` and `licence`, so signatures cover them. | M | 1 |
| AI02-F031 | Unsigned result quarantine | A fresh import is unsigned. It stays in "Quarantine" and cannot be granted to apps until signed under the trust policy. | M | 1 |
| AI02-F032 | Import adapters | LoRA from PEFT safetensors into an adapter `.fpk` (`<weight>.lora_a/.lora_b`, `lora_alpha`). FORGE has the load path, not a CLI converter; the converter is Proposed. | C | 3 |
| AI02-F033 | Import cache view | Shows FORGE's import cache (`$FORGE_CACHE`) entries and lets the user delete them. | S | 2 |
| AI02-F034 | Disk space check | A job is refused before start if the target has less free space than the estimate plus 10%. | M | 1 |

### 4.3 Inspect and verify

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| AI02-F035 | Overview | Manifest fields: `architecture`, `session`, `template`, `codec`, token ids, `max_position_embeddings`, `add_bos`, format version. | M | 1 |
| AI02-F036 | Section table | Name, offset, size, BLAKE3 prefix (`forge inspect`). | M | 1 |
| AI02-F037 | Tensor list | `--tensors`: name, codec, shape, bytes; filter and sort; total by codec. | S | 1 |
| AI02-F038 | Blueprint view | `--blueprint`: graph text in `codeEdit` (read-only) with search. | S | 2 |
| AI02-F039 | Graph view | The blueprint as a `nodeGraph`, read-only, collapsed by `repeat` block. | C | 3 |
| AI02-F040 | Verify hashes | `--verify` / `forge verify`: every section and tensor hash; result per section; runs as a job for large files. | M | 1 |
| AI02-F041 | Signature report | Per signer: key id, key name from the trust store, valid or bad, trusted or not, role. | M | 1 |
| AI02-F042 | Encrypted files | Shows "sealed"; metadata readable without the key; tensor hashes checked only when the key is available. | M | 2 |
| AI02-F043 | Tokenizer view | Vocabulary size, BOS and EOS, a try-it box that tokenises typed text. | C | 2 |
| AI02-F044 | Dump tensor | `forge dump`: export one tensor as raw little-endian f32 through the save portal. | C | 3 |
| AI02-F045 | Tensor statistics | Min, max, mean, histogram of one tensor (`chart` Bar); computed in the job worker. | C | 3 |
| AI02-F046 | Model card viewer | The `card` section in `markdownView`; licence text in a second tab; first-use acknowledgement recorded per user. | M | 1 |
| AI02-F047 | Compare two files | Section and tensor diff of two `.fpk`: added, removed, changed hash, changed codec. | C | 3 |
| AI02-F048 | Detached signatures | Verify any file with a `.sig` (`forge verify <file> --sig`): shows the bound name, digest and signers. | S | 2 |

### 4.4 Signing, encryption, trust

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| AI02-F049 | Sign model | Picks a signing key held in SE04; the signature is made by `svc.keyring` (`forge sign --signer`, FORGE-WP-33.3). No private key file is written. | M | 1 |
| AI02-F050 | Counter-sign | Adds a second signature to a signed file when the digest still matches. | S | 2 |
| AI02-F051 | Create key hand-off | "New signing key…" opens SE04 with role `model`; the public key comes back into the trust store view. Replaces `forge keygen` writing `NAME.key`. | M | 1 |
| AI02-F052 | Import key files | Existing `NAME.key` / `NAME.pub` pairs can be imported into SE04; the file is then offered for secure erase (SE05). | S | 2 |
| AI02-F053 | Trust store view | Keys with id, name, roles, validity, pins, revoked state (FORGE-WP-33.1). | M | 1 |
| AI02-F054 | Trust a key | Add a public key (64 hex, `.pub` file, or QR) with role `model`; needs administrator authentication (SH03). | M | 1 |
| AI02-F055 | Trust policy | Rules: signed models only (always on in the app profile); which keys; require N of M signers; per-class key sets; allow self-signed imports for this user only. | M | 1 |
| AI02-F056 | No "any valid signature" mode | The empty-key-list behaviour of FORGE is not offered (FORGE-WP-29.3). | M | 1 |
| AI02-F057 | Revocation | Shows the signed revocation list and its counter; a revoked key turns its models to "untrusted" and unloads them. | M | 2 |
| AI02-F058 | Encrypt model | `forge encrypt`: seals tensor data. The 32-byte key is created and stored in `svc.keyring`, not in a `.mkey` file; the service receives it over a descriptor at load. | S | 2 |
| AI02-F059 | Decrypt for export | Makes an unsealed copy after authentication; logged. | C | 3 |
| AI02-F060 | Sign detached | Sign any file (adapter source, evaluation set, script) with `--detached`; `--name` to bind another name. | C | 3 |
| AI02-F061 | Policy check view | `forge security check --json` for the service's policy: each control's state, backend and verdict in a table. | S | 2 |
| AI02-F062 | Trust change audit | Every key added, removed, policy edited goes to `svc.audit`. | M | 1 |
| AI02-F063 | Tamper state | A model whose hash check fails is marked "tampered", unloaded, and cannot be granted; the user can restore from a snapshot (AD03). | M | 1 |

### 4.5 Download

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| AI02-F064 | Off by default | No network capability is used until the user opens "Download…". There is no built-in catalogue and no vendor account. | M | 2 |
| AI02-F065 | Download by URL | The user pastes one or more HTTPS URLs. First contact with each host raises the shell's `net.user` prompt. | M | 2 |
| AI02-F066 | Expected checksum | SHA-256 or BLAKE3 field per file; a mismatch deletes nothing but quarantines the file as "checksum failed". | M | 2 |
| AI02-F067 | Signature after download | If the file is a signed `.fpk` or has a `.sig`, it is verified against the trust store before it can leave quarantine. | M | 2 |
| AI02-F068 | Resume | HTTP range requests; partial files keep a rolling hash state; resume after restart. | M | 2 |
| AI02-F069 | Route choice | Clear, VPN or Tor per download (`net.route`). | S | 2 |
| AI02-F070 | Manifest file | A small JSON list (URL, size, hash) can be opened to queue a multi-file model. Parsed in the helper. | S | 3 |
| AI02-F071 | Bandwidth and schedule | Rate limit; "only on mains power"; pause and resume. | C | 3 |
| AI02-F072 | Network badge | `[net]` in the header while a download runs; click lists hosts and bytes. | M | 2 |
| AI02-F073 | Mirror list | The user may keep named base URLs (own server); nothing is preset. | C | 3 |

### 4.6 Hardware

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| AI02-F074 | CPU panel | Name, feature flags, the SIMD table in use (`forge info` lines `cpu`, `features`, `lane`), cores and hybrid layout. | M | 1 |
| AI02-F075 | GPU runtimes panel | One row per runtime (`cpu`, `cuda`, `hip`, `level_zero`, `vulkan`, `metal`): library loads or not. The wording is "library found", never "works", as `forge info` only checks loading. | M | 1 |
| AI02-F076 | Device test | Runs a short oracle check on a chosen device inside the model worker and reports pass or fail with the driver's message. | S | 3 |
| AI02-F077 | Memory panel | Physical and available RAM, `RLIMIT_AS`, the host budget, soft watermark, and where the limit came from. | M | 1 |
| AI02-F078 | Budget editor | Sets the service's host limit (size or percentage), soft fraction, device pool limit. Maps to `FORGE_MEM_LIMIT`, `FORGE_MEM_SOFT`, `FORGE_MEM_DEVICE_LIMIT`. Needs administrator authentication. | M | 1 |
| AI02-F079 | Live pools | Per pool: limit, used, peak, cache bytes, by-tag bars (`kv`, `weights`, `resident`, `scratch`), denials, evictions. | S | 2 |
| AI02-F080 | Device choice | Default device per class (CPU default; a GPU only if the GPU question of FORGE-WP-32.2 is answered). | S | 3 |
| AI02-F081 | Fit estimate | For each model: weights mapped, KV per 1 000 tokens for the chosen KV codec, "fits / tight / does not fit". | S | 2 |
| AI02-F082 | KV codec setting | `f32`, `int8`, `fp8`, `tbq4`, `tbq3`, `tbq4p`, `tbq3p`, `k8v4` per model, with the note that `tbq*p` run on the CPU only. | C | 3 |
| AI02-F083 | Power profile | Service behaviour on battery: allow, defer background, refuse above N watts (through `svc.power`). | S | 2 |
| AI02-F084 | NUMA and pinning | Advanced: NUMA node and performance-core pinning for workers (`--numa-node`, `--no-pin`). | C | 3 |
| AI02-F085 | Huge pages | Toggle `FORGE_HUGEPAGES` for weight mappings. | C | 3 |

### 4.7 Run, benchmark, evaluate

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| AI02-F086 | Test run | A plain prompt box on the detail page: `forge run` options `-n`, `--temp`, `--top-k`, `--top-p`, `--min-p`, `--repeat-penalty`, `--seed`, `--ctx`, `--no-bos`, `--ids`. Output and timing (prompt and new tokens, tokens per second, plans compiled and reused). Not a chat; no history. | M | 1 |
| AI02-F087 | Token-id input | `--tokens 1,2,3` mode for models without a tokenizer. | C | 2 |
| AI02-F088 | Grammar test | `--grammar FILE.gbnf`: paste a grammar and check the output is constrained. | S | 2 |
| AI02-F089 | Speculative decoding test | `--draft`, `--lookup`, `--ngram`, `--ngram-min`, `--spec-k`; shows acceptance rate and tokens per pass. | C | 3 |
| AI02-F090 | One-shot graph run | For non-generative models: bind inputs (`--input name=file.f32`), run, save outputs (`--out PREFIX`). | C | 3 |
| AI02-F091 | Logits export | `--logits FILE`: write prefill logits for later comparison. | C | 3 |
| AI02-F092 | Benchmark | `forge bench -p -n --warmup`: prefill and decode tokens per second; repeated N times with mean and spread. | M | 1 |
| AI02-F093 | Kernel benchmark | `--gemm N`: per SIMD table and GPU primitives. | S | 2 |
| AI02-F094 | Driver call counts | On CUDA/HIP devices: kernels launched, graph launches, copies and syncs per token, as `forge bench` prints. | C | 3 |
| AI02-F095 | Benchmark history | Every run stored with model hash, device, FORGE version, power state, thermal state, settings. | M | 1 |
| AI02-F096 | Comparison view | Pick 2–6 runs: bars for prefill and decode speed, memory, size; table of deltas. | M | 2 |
| AI02-F097 | Honest labels | Runs on battery, under thermal throttling or with other models loaded are flagged; flagged runs are excluded from "best" by default. | M | 2 |
| AI02-F098 | Quantisation drift | Compares prefill logits of two codecs of one family on a fixed prompt set: maximum and mean absolute difference, top-1 agreement. | S | 3 |
| AI02-F099 | Evaluation sets | JSONL files (prompt, expected, scorer) added by the user; hashed and optionally signed. No set ships by default. | S | 3 |
| AI02-F100 | Scorers | Exact match, regular expression, JSON Schema conformance, multiple choice, perplexity on a text file (from `forward_all` logits). | S | 3 |
| AI02-F101 | Evaluation run | A job with fixed seed and temperature 0; result table with a confidence interval; reproducibility record (model hash, set hash, FORGE version). | S | 3 |
| AI02-F102 | Determinism check | Runs the same prompt twice on the CPU path and reports whether outputs are identical. | C | 3 |
| AI02-F103 | Export results | CSV and JSON; a Markdown report through the save portal. | S | 2 |

### 4.8 Permissions and scheduling (`svc.model` administration)

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| AI02-F104 | App × model matrix | Rows are apps that declare `ai.local`; columns are classes and models; cells are allow, ask, deny. | M | 1 |
| AI02-F105 | Per-class scopes | An app is granted classes (text generation, embeddings, speech) and then "any trusted model of the class" or named models. | M | 1 |
| AI02-F106 | First-use prompt | When an app first asks for a class it lacks, the shell prompts; the choice appears here and in AD05. | M | 1 |
| AI02-F107 | Revoke | Immediate: the service cancels that app's running requests (SI-6). | M | 1 |
| AI02-F108 | Quotas | Per app: maximum concurrent requests, tokens per minute, maximum context, background-only flag. | S | 2 |
| AI02-F109 | Priority classes | Interactive, foreground, background per app (FORGE-WP-29.2). | S | 2 |
| AI02-F110 | Loaded models | Live list: model, worker pid, device, memory, sessions, idle time; Unload. | M | 1 |
| AI02-F111 | Queue view | Waiting and active requests by app with class and age; no prompt text is shown or available. | M | 1 |
| AI02-F112 | Idle unload | Time after which an unused model is unloaded; pin a model to stay loaded. | M | 1 |
| AI02-F113 | Preload at login | Optional list of models to load when the session starts. | C | 2 |
| AI02-F114 | Usage history | Per app and model: requests, tokens in and out, time; counts only (FORGE-WP-29.4). | S | 2 |
| AI02-F115 | Prefix-cache policy | Per-app partitioning is fixed on; the user sets the number of cached prefixes per app. | S | 2 |
| AI02-F116 | KV spill policy | Off by default with the reason shown; if enabled, the spill directory must be on an encrypted dataset and is wiped at logout. | M | 2 |
| AI02-F117 | Service health | State of the scheduler and each worker, restarts, last error; Restart service (administrator). | M | 1 |
| AI02-F118 | Adapter attach | Attach adapter files to a base model; apps see them as selectable variants. | C | 3 |
| AI02-F119 | Adapter compatibility check | Checks that every adapter tensor names a weight of the base and that shapes match, before attaching. | C | 3 |

### 4.9 Serving and remote devices (optional component)

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| AI02-F120 | Not installed by default | `forge-serve` is absent on the desktop (FORGE build plan Q15). The Serving page explains this and offers to install the add-on through SY05. | M | 2 |
| AI02-F121 | Endpoint editor | Model, socket (Unix path by default once FORGE-WP-32.1 lands; TCP only on request), `--max-active`, `--ctx`, `--mem-limit`, `--max-queue`, `--kv-spill`, `--no-pin`, `--numa-node`. | S | 2 |
| AI02-F122 | Listen prompt | A TCP listener needs an explicit grant naming address and port; non-loopback needs a second confirmation and shows that the server has no authentication or TLS. | M | 2 |
| AI02-F123 | Start, stop, restart | Endpoints run as supervised helpers; state survives closing the window only if "Keep running" is ticked (`bg.run`). | S | 2 |
| AI02-F124 | Endpoint dashboard | From `/metrics`: active, waiting, completed, rejected, prefix hits, tokens reused, KV evictions and spills, memory by pool; tokens per second derived from step counters. `chart` Line per series. | S | 2 |
| AI02-F125 | Try endpoint | Sends `/health`, `/v1/models` and a one-token completion; shows status and latency. | S | 2 |
| AI02-F126 | Proxy editor | `forge-serve proxy`: backends, routes (`/prefix=host:port,…`), `--mode l7\|l4`, `--strategy` (`rr`, `wrr`, `lc`, `ewma`, `hash`, `p2c`, `kv`), `--retries`, `--health-ms`. | C | 3 |
| AI02-F127 | Backend table | Per backend: healthy, breaker state (closed, open, half-open), in flight, requests, failures, EWMA latency, weight. | C | 3 |
| AI02-F128 | Proxy limits shown | States that the proxy has no TLS and delivers streamed answers only at the end. | C | 3 |
| AI02-F129 | Lend a device (`rpc-serve`) | Editor for `--bind`, `--port`, `--device`, tokens (from the keyring, via `--token-file` on a private descriptor), `--authorized-keys`, `--pubkey`, TLS certificate and key, `--tls-client-ca`, `--tls-client-map`, `--quiet`. | C | 3 |
| AI02-F130 | Remote scripts off | `--allow-scripts` and its options (`--script-timeout`, `--script-mem-mb`, `--script-no-proc/-fs/-net`) are shown but locked off in the desktop profile; an administrator override is logged. | C | 3 |
| AI02-F131 | No open server | The editor refuses to start `rpc-serve` without tokens or keys, on any address. | M | 3 |
| AI02-F132 | Remote devices | List of `rpc:host:port` devices this machine may use: address, authentication (token or key from the keyring), TLS with CA or pinned fingerprint, server name; test connection. | C | 3 |
| AI02-F133 | Connection log | Principals, permissions, refusals from the server's security log, in a table. | C | 3 |
| AI02-F134 | Policy file per server | `--policy`: the generated policy is shown read-only; the listen address must pass `net_listen`. | S | 3 |

### 4.10 Disk, jobs, housekeeping

| ID | Feature | Detail | Pri | Ph |
|---|---|---|---|---|
| AI02-F135 | Disk usage | Bars per model family, import cache, spill directory, benchmark data, downloads in progress; free space. | M | 1 |
| AI02-F136 | Clean up | Delete import cache, partial downloads, old benchmark logs; preview of what is freed. | M | 1 |
| AI02-F137 | Jobs view | All jobs: kind, model, state, progress, started, duration, log; cancel, retry, open result. | M | 1 |
| AI02-F138 | Job limits | One heavy job at a time by default; jobs yield to interactive model use. | S | 2 |
| AI02-F139 | Notifications | Job done or failed, download done, model quarantined, key revoked. | S | 1 |
| AI02-F140 | Export model | Copy a library `.fpk` out through the save portal, with its card, licence and signatures. | S | 2 |
| AI02-F141 | Library backup rules | Declares to `svc.backup`: metadata always; model files optional (large). | S | 2 |
| AI02-F142 | Snapshot restore | Offers the last good ZFS snapshot of a tampered or deleted model (hand-off to AD03). | C | 3 |
| AI02-F143 | Command preview | Every action shows the equivalent `forge` command line (copyable) so users can learn and script it. | S | 2 |
| AI02-F144 | First-run page | Explains: signed models only, nothing downloads by itself, where to get a model. | M | 1 |
| AI02-F145 | Diagnostics | Export versions, hardware summary, service state and job logs; no prompts, no outputs. | S | 2 |

## 5. GUI

### 5.1 Window map

```
Model Manager
├─ Main window (sidebar + content + inspector)
│  ├─ Sidebar (F9)
│  │  ├─ Library: All models, by class, Quarantine, Adapters
│  │  ├─ Running: Loaded models, Queue
│  │  ├─ Permissions
│  │  ├─ Trust: Keys, Policy, Revocations
│  │  ├─ Hardware: CPU and GPU, Memory budget
│  │  ├─ Benchmarks, Evaluations
│  │  ├─ Serving (optional add-on): Endpoints, Proxy, Lent devices, Remote devices
│  │  ├─ Jobs
│  │  └─ Disk
│  ├─ Content: the selected page
│  ├─ Inspector (F4): properties of the selected model, key, app, job or endpoint
│  └─ Status bar: service state, loaded models, memory used / budget, running jobs, [net]
├─ Model detail page (tabs: Overview, Card, Licence, Sections, Tensors, Blueprint, Signatures,
│                     Provenance, Test run, Benchmarks, Used by)
├─ Wizards
│  ├─ Import model (Source → Options → Card and licence → Review → Job)
│  └─ New endpoint (Model → Socket → Limits → Review)
├─ Dialogs
│  ├─ Sign model
│  ├─ Trust a key
│  ├─ Encrypt model
│  ├─ Download by URL
│  ├─ Re-quantise
│  ├─ Remove model
│  ├─ Edit memory budget
│  ├─ App permissions (per app)
│  ├─ Compare benchmarks
│  └─ Preferences (Ctrl+,)
├─ Popovers: trust badge details, fit estimate, command preview, network hosts
└─ Shell-drawn: file and folder portals, administrator authentication (SH03), network first
   contact, listen grant, key use confirmation from SE04
```

### 5.2 Main window

Shown at 1280×800 on the Library page with one model selected.

```
+----------------------------------------------------------------------------------------+
| [=] Model Manager                       [ Search / commands  Ctrl+K ]        [▁][▢][✕] |(1)
+---------------+------------------------------------------------------+-----------------+
| LIBRARY       | [Import…] [Download…]  [v All classes] [v Trust: any]| qwen3-8b        |(7)
| > All      12 | [ Filter…            ]            (list) (grid)   (3)| q4_k_m          |
|   Text      7 +------------------------------------------------------+ ✔ Signed, trusted|
|   Embedding 1 | Name            Class  Codec   Size   Trust    Loaded| by "Odin 2026"  |
|   Speech    2 | ▾ qwen3-8b      text           3 files               |  3f9a12c4       |
|   Plugin    1 | >   q4_k_m      text   q4_k_m  5.1 GB ✔ trusted ●CPU |-----------------|
|   Quarantine 1|     q8_0        text   q8_0    8.6 GB ✔ trusted      | Architecture    |
|   Adapters  0 |     f16         text   f16    16.2 GB ✔ trusted      |  qwen3          |
| RUNNING       |   llama-3b      text   q5_k_m  2.3 GB ✔ trusted      | Context 32 768  |
|   Loaded    1 |   embed-small   embed  f16     0.3 GB ✔ trusted      | Tensors 291     |
|   Queue     0 |   whisper-like  speech q8_0    0.9 GB ⚠ unknown key  | Hash b3:91c2…   |
| PERMISSIONS   |   mistral-7b    text   q4_k    4.4 GB ✖ unsigned  (4)| Licence Apache-2|
| TRUST         |   …                                                  | Fits: yes (8k)  |
|   Keys        |                                                      |-----------------|
|   Policy      |                                                      | Used by         |
| HARDWARE      |                                                      |  Assistant      |
|   CPU and GPU |                                                      |  Notebook       |
|   Memory      |                                                      |-----------------|
| BENCHMARKS    |                                                      | [Open details]  |
| EVALUATIONS   |                                                      | [Test run]      |
| SERVING       |                                                      | [Benchmark]     |
| JOBS        1 |                                                      | [Sign…] [Verify]|
| DISK      (2) |                                                   (5)| [Remove…]       |
+---------------+------------------------------------------------------+-----------------+
| ● svc.model running · 1 loaded · mem 5.1 / 12.0 GB · 1 job: import mistral-7b 42%   (6)|
+----------------------------------------------------------------------------------------+
```

1. **Header bar.** Menu, title, command palette. `[net]` appears only during a download or while a TCP endpoint listens.
2. **Sidebar, 220 px.** `treeView` with counts. Serving is greyed with "not installed" when the add-on is absent.
3. **Toolbar.** Primary action Import; Download; class and trust filters (`comboBox`); filter `lineEdit`; list/grid switch.
4. **Model table.** `treeView` in table mode (families expand to files). Trust column uses the system trust badge: trusted (`--ok`), unknown key (`--warn`), unsigned or tampered (`--danger`), always with a word.
5. **Content area.** Replaced by the page picked in the sidebar.
6. **Status bar.** Service state, loaded count, memory against budget (click opens Memory), running jobs (click opens Jobs).
7. **Inspector, 280 px.** `propertyGrid` of the selection plus action buttons. Actions that the trust state forbids are disabled with the reason in a tooltip.

States: empty library (one line "No models yet. Models must be imported and signed before apps can use them." and an Import button), service not running (banner with Start, library still browsable read-only), loading list, verify in progress (spinner in the trust cell), tampered (row in `--danger`, actions limited to Details, Restore, Remove), library location missing, read-only (non-administrator: policy pages visible but locked), narrow (sidebar overlay, inspector as a sheet, table keeps Name, Size, Trust).

### 5.3 Other views and dialogs

#### Import wizard, Options step (760×560)

```
+------------------------------------------------------------------------------+
| Import model                           Source › Options › Card › Review      |
+------------------------------------------------------------------------------+
| Source   ~/Downloads/qwen3-8b/  (Hugging Face directory, 16.2 GB, qwen3)     |
|                                                                              |
| Weight codec   [v q4_k_m — mixed 4-bit, ≈5.1 GB ]   Recommended for 12 GB ✔  |
| Embedding      [v q8_0 (default for quantised) ]                             |
| Template       [v Auto (qwen3) ]                                             |
| ▾ Mixed precision                                                            |
|   Recipe  (•) forge  ( ) llama        [ ] Pure (one codec)  [ ] bf16 head    |
|   Output head [v default]   Token embedding [v default]                      |
|   Per-tensor overrides                                                       |
|   | Pattern (regex)        | Type  | Matches |                               |
|   | layer\.0\..*           | q6_k  |   12    |   [+] [−]                     |
|                                                                              |
| Result  ≈5.1 GB · free on library disk 212 GB                                |
| Command forge import … -o qwen3-8b.q4_k_m.fpk --codec q4_k_m        [Copy]   |
|                                                   [Back] [Cancel] [Next]     |
+------------------------------------------------------------------------------+
```

For a `.gguf` source the codec block is replaced by "Tensors are copied as stored (Q4_K)". For `.onnx` the step lists inputs, outputs and any unsupported op. States: analysing source, unsupported architecture (FORGE's message, no Next), not enough disk space.

#### Model detail, Signatures tab

```
+----------------------------------------------------------------------------------------+
| ‹ Library   qwen3-8b · q4_k_m                                   [Test run] [Benchmark] |
| Overview | Card | Licence | Sections | Tensors | Blueprint |>Signatures<| Provenance | …|
+----------------------------------------------------------------------------------------+
| Integrity   ✔ all 6 sections and 291 tensors match their hashes    checked today 10:02 |
|             [Verify again]                                                             |
| Signatures                                                                             |
|  Key id     Name            Role    Signature   Trusted          Added                 |
|  3f9a12c4   Odin 2026       model   ✔ valid     ✔ in trust store 2 Nov                 |
|  77b0e2a1   (unknown key)   —       ✔ valid     ⚠ not trusted    —        [Trust key…] |
| Policy      "2 of: Odin 2026, Build host"  →  ✖ not met (1 of 2)                       |
| Encryption  not sealed                                             [Encrypt…]          |
|                                                    [Sign with my key…] [Export .pub…]  |
+----------------------------------------------------------------------------------------+
```

`tabWidget`; tables are `tableView`. States: verifying (progress with bytes hashed), hash mismatch (names the failing sections), sealed without key ("tensor hashes not checked"), no signatures.

#### Permissions page

```
+----------------------------------------------------------------------------------------+
| Permissions                         [v Show: apps with ai.local]      [Open Permissions]|
+----------------------------------------------------------------------------------------+
| App            Text generation       Embeddings        Speech-to-text   Quota   Priority|
| Assistant      [v any trusted   ]    [v any trusted ]  [v ask        ]  default interact|
| Notebook       [v llama-3b only ]    [v deny        ]  [v deny       ]  2 req   foregrnd|
| Screen Reader  [v deny          ]    [v deny        ]  [v deny       ]  —       —       |
| Sound Recorder [v deny          ]    [v deny        ]  [v any trusted]  1 req   backgrnd|
| Search service [v deny          ]    [v embed-small ]  [v deny       ]  4 req   backgrnd|
+----------------------------------------------------------------------------------------+
| Assistant — last 7 days: 412 requests · 1.9 M tokens in · 0.3 M out   [Revoke all]      |
+----------------------------------------------------------------------------------------+
```

Each cell is a `comboBox`: deny, ask, any trusted model of the class, or named models (multi-select popup). Changes apply at once. States: no app has asked yet (explanation line), service down (read-only with last known values).

#### Hardware page

```
+----------------------------------------------------------------------------------------+
| Hardware                                                              [Refresh  F5]    |
+--------------------------------------------+-------------------------------------------+
| CPU                                        | Memory                                    |
|  AMD Ryzen 9 7950X · 16 cores              |  Physical 64.0 GB · available 41.2 GB     |
|  Features  avx2 avx512f avx512bw fma f16c… |  Process limit (RLIMIT_AS)  none          |
|  SIMD table in use  avx512                 |  Model budget  12.0 GB  (set here)        |
|--------------------------------------------|  Soft watermark 90%          [Edit…]      |
| Compute runtimes        Library            |  [##########----------] 5.1 / 12.0 GB     |
|  cpu                    ✔ built in         |  kv 0.4 · weights mapped 5.1 (not charged)|
|  vulkan                 ✖ not found        |  denials 0 · evictions 2                  |
|  cuda                   ✖ not found        |-------------------------------------------|
|  hip                    ✖ not found        | On battery                                |
|  level_zero             ✖ not found        |  (•) allow  ( ) defer background ( ) refuse|
|  metal                  — not on this OS   | Idle unload after [ 10 ] min              |
| "Library found" does not mean a device     |                                           |
|  works. [Run device test…]                 |                                           |
+--------------------------------------------+-------------------------------------------+
```

Two `panel` columns of labels and one `progressBar`. States: service down (static probe only), device test running, device test failed (driver message shown verbatim).

#### Compare benchmarks dialog (900×560)

```
+------------------------------------------------------------------------------------+
| Compare benchmarks                                              [Export CSV…]      |
+------------------------------------------------------------------------------------+
| Run                     Device  Prefill tok/s  Decode tok/s  Mem GB  Size GB  Flags|
| qwen3-8b q4_k_m  2 Nov  CPU        212 ±4        11.2 ±0.3     5.6     5.1         |
| qwen3-8b q8_0    2 Nov  CPU        148 ±3         7.9 ±0.2     9.2     8.6         |
| qwen3-8b q4_k_m  5 Nov  CPU        171 ±9         8.8 ±0.9     5.6     5.1   ⚠ batt|
+------------------------------------------------------------------------------------+
| Decode tok/s                                                                       |
|  q4_k_m 2 Nov  ██████████████████████  11.2                                        |
|  q8_0   2 Nov  ███████████████          7.9                                        |
|  q4_k_m 5 Nov  █████████████████        8.8  (flagged: on battery)                 |
| Settings: -p 32 -n 32 --warmup 1 · 5 repeats · FORGE 0.1.0                         |
|                                                                        [Close]     |
+------------------------------------------------------------------------------------+
```

`tableView` plus `chart` (Bar). The numbers in this wireframe are placeholders that show layout, not measurements.

#### Serving: endpoint dashboard (add-on installed)

```
+----------------------------------------------------------------------------------------+
| Endpoints                                         [New endpoint…]                      |
+------------------------+---------------------------------------------------------------+
| > local-chat   ● up    | local-chat — qwen3-8b q4_k_m                 [Stop] [Restart] |
|   lan-share    ○ off   | Socket  unix:$XDG_RUNTIME_DIR/org.pbsd.model-manager/chat.sock|
|                        | Auth    peer credentials (same user)      TLS  n/a (local)    |
|                        |---------------------------------------------------------------|
|                        | Active 2 / 8   Waiting 0   Completed 1 204   Rejected 0       |
|                        | Prefix hits 311 · tokens reused 88 412 · KV evictions 4       |
|                        | Tokens/s  ▁▂▅▇▆▅▃▂▁▁▂▆▇▇▅  (last 10 min)                      |
|                        | Memory    [#######-----] 6.2 / 12.0 GB                        |
|                        |---------------------------------------------------------------|
|                        | Limits  max-active 8 · ctx model max · max-queue 32 · spill off|
|                        | [Try endpoint]   health ok · models ok · 1-token 84 ms        |
+------------------------+---------------------------------------------------------------+
```

`listView` left; right side labels, `chart` (Line) and `progressBar`. An endpoint on a TCP address shows a `--net` banner "Listening on 192.168.1.20:8080 — no authentication, no TLS" that cannot be dismissed. States: add-on not installed (explanation and "Install serving add-on"), stopped, starting, failed to bind, policy refused the address.

#### Trust a key dialog (520×360)

```
+----------------------------------------------------------+
| Trust a signing key for models                           |
+----------------------------------------------------------+
| Public key  [ 9c41…e07b (64 hex)            ] [From file…]|
| Key id      3f9a12c4                                      |
| Name        [ Odin 2026                     ]             |
| Role        model (fixed)                                 |
| Valid until [ none            v]                          |
| Apply to    (•) all classes  ( ) only [v Text generation ]|
| ⚠ Models signed by this key can be granted to apps.       |
|                          [Cancel] [Trust key (authenticate)]|
+----------------------------------------------------------+
```

#### Jobs page

```
+----------------------------------------------------------------------------------------+
| Jobs                                             [v All kinds] [Clear finished]        |
+----------------------------------------------------------------------------------------+
| Kind      Model            State      Progress              Started   Duration         |
| import    mistral-7b       running    [#####-------] 42%    10:31     00:04:12 [Cancel]|
| verify    qwen3-8b f16     done       ✔ 291 tensors         10:02     00:00:41         |
| bench     llama-3b q5_k_m  done       ✔ 5 repeats           09:40     00:01:10 [Open]  |
| download  whisper-like     paused     [########----] 71%    Mon       —        [Resume]|
+----------------------------------------------------------------------------------------+
| Log (import mistral-7b)                                                                |
| 10:31:02 reading config.json … 10:31:04 template llama … 10:35:10 tensor 180/291 …     |
+----------------------------------------------------------------------------------------+
```

### 5.4 Menus

Items are separated by `·`; `—` marks a separator.

```
File     Import Model… Ctrl+I · Download by URL… Ctrl+Shift+D · Open File for Inspection… Ctrl+O
         — Export Model… Ctrl+Shift+S · Export Benchmark Results… · Export Diagnostics…
         — Close Ctrl+W · Quit Ctrl+Q
Edit     Undo Ctrl+Z · Redo Ctrl+Shift+Z — Copy Ctrl+C · Copy Hash Ctrl+Shift+C · Copy Command Ctrl+Alt+C
         — Select All Ctrl+A · Find Ctrl+F — Preferences Ctrl+,
View     Sidebar F9 · Inspector F4 · List / Grid Ctrl+1 / Ctrl+2 · Group by Family Ctrl+G
         — Go to Library Ctrl+L · Loaded Models Ctrl+R · Permissions Ctrl+Shift+P · Jobs Ctrl+J
         — Refresh F5 · Zoom In Ctrl++ · Zoom Out Ctrl+- · Reset Zoom Ctrl+0 · Full Screen F11
Model    Open Details Enter · Test Run Ctrl+T · Benchmark Ctrl+B · Compare… Ctrl+Shift+B
         — Verify Ctrl+Shift+V · Sign… Ctrl+Shift+G · Encrypt… · Re-quantise… · Attach Adapter…
         — Set as Default for Class · Rename F2 · Tags… · Remove… Delete
Service  Loaded Models · Unload Selected Ctrl+U · Unload All · Queue
         — Memory Budget… Ctrl+M · Idle Unload… · Power Behaviour…
         — Restart Model Service… (administrator)
Trust    Keys · Trust a Key… · New Signing Key (Passwords and Keys)… · Policy… · Revocations
         — Policy Check
Serving  New Endpoint… · Start · Stop · Restart · Try Endpoint
         — Proxy… · Lend a Device… · Remote Devices… — Install Serving Add-on…
Help     Model Manager Help F1 · About Signed Models · Codec Guide · Keyboard Shortcuts · About
```

### 5.5 Keyboard and pointer

| Keys | Action |
|---|---|
| `Enter` | Open details of the selected model |
| `Space` | Quick look: card and trust summary in a popover |
| `Ctrl+I` | Import |
| `Ctrl+T` / `Ctrl+B` | Test run / benchmark the selection |
| `Ctrl+Shift+V` | Verify hashes and signatures |
| `Ctrl+U` | Unload the selected loaded model |
| `Alt+Left` / `Alt+Right` | Back / forward between pages |
| `Ctrl+Tab` | Next tab on the detail page |

Context menus:

- Model row: Open Details, Test Run, Benchmark, Verify, Sign, Encrypt, Re-quantise, Set as Default, Permissions for This Model, Copy Hash, Copy Path, Export, Show in Files, Remove.
- Loaded model: Unload, Pin Loaded, Show Sessions, Open Details.
- Key row: Rename, Limit to Class, Show Models Signed by This Key, Remove Trust.
- App row (Permissions): Revoke All, Set Quota, Set Priority, Show Usage, Open in Permissions (AD05).
- Job row: Cancel, Retry, Open Result, Copy Log.

Drag and drop: `.fpk`, `.gguf`, `.onnx` files or a model folder onto the window start the import wizard (never a direct load); a `.pub` file onto Trust opens "Trust a key"; a model row dragged out offers a file handle (export, after confirmation); benchmark rows dragged onto each other open Compare.

### 5.6 States

| State | Behaviour |
|---|---|
| Empty | Library explains signed-only policy; Import button. |
| Loading | Skeleton rows; the list comes from the service in one call. |
| Service down | Banner with Start; pages that need the service are read-only. |
| Not administrator | Budget, policy, trust and service restart are locked with a padlock and "Authenticate to change". |
| Offline | Normal. Download shows "No route" only when used. |
| Tampered model | Marked, unloaded, grants suspended (not deleted), Restore offered. |
| Quarantine | Fresh imports and failed downloads; usable only for Inspect, Verify, Test run (in the job worker, not through `svc.model`), Sign, Remove. |
| Job running | Status bar progress; closing the window keeps jobs running in the service. |
| Narrow (<640) | Sidebar overlay; inspector sheet; wizard steps stack. |

### 5.7 Widgets

| Region | FUSE widget |
|---|---|
| Sidebar | `treeView` |
| Library | `treeView` (table mode), `gridView`, `comboBox`, `lineEdit`, `toolButton` |
| Inspector | `propertyGrid`, `button`, `label` |
| Detail tabs | `tabWidget`, `tableView`, `markdownView`, `codeEdit` (read-only), `nodeGraph` |
| Permissions | `tableView` with `comboBox` cells |
| Hardware, budgets | `panel`, `label`, `progressBar`, `slider`, `spinBox`, `radioButton` |
| Benchmarks, dashboard | `chart` (Bar, Line), `tableView` |
| Wizards, dialogs | `dialog`, `lineEdit`, `comboBox`, `checkBox`, `fileDialog` is not used (portal instead) |
| Jobs | `tableView`, `progressBar`, `codeEdit` (log, read-only) |

Missing: `AI02-GAP-1` editable cell widgets inside `tableView` (combo box cells for the permission matrix); `AI02-GAP-2` error bars and a horizontal bar mode for `chart` (`ChartKind` is Line, Bar, Area, Scatter only: `FUSE:Source/FUSE/UI/Widgets/include/fuse/ui/widgets/widgets.hpp:244`); `AI02-GAP-3` a sparkline cell renderer.

## 6. Architecture

### 6.1 Components

| Component | Language | Process | Responsibility |
|---|---|---|---|
| App logic (`main.fs`, `src/*.fs`) | ForgeScript | app host | Pages, view models, wizards, command preview, admin client of `svc.model` |
| UI (`ui/*.fuseui`, `style/*.css`) | FUSE UI | app host | Windows, pages, dialogs |
| Model service `forge-modeld` | C++23 (FORGE-WP-29) | system service: scheduler plus one worker per loaded model | Loads and runs models, enforces permissions and budgets. Built in FORGE; administered here |
| Job worker `model-job` | C++23, links FORGE `modelfile`, `import`, `inference` | disposable helper per job, capability mode | Import, re-quantise, verify, dump, tensor statistics, benchmark, evaluation, quarantine test runs. Parses untrusted model files (PLATFORM.md section 2 reason a) |
| Download helper `fetch-helper` | C++23 | helper, only while downloading | HTTPS with range resume, rolling hashes; holds the only sockets (reason a) |
| Serving supervisor `serve-supervisor` | C++23 | helper, only with the add-on | Starts and watches `forge-serve`, `forge-serve proxy`, `forge rpc-serve`; scrapes `/metrics`; generates each server's policy file (reason d) |
| Library index | SQLite through `SqliteModel` | inside `forge-modeld` (authoritative) with a read-only view in the app | Model records, provenance, grants, usage counts |

```
┌──────────── app host: org.pbsd.model-manager ────────────┐
│ FUSE UI ◄─ view models ─ ForgeScript ── session bus ─────┼──► svc.model (user interface)
└───┬───────────┬───────────────┬──────────────────────────┘        ▲
    │           │               │  svc.model.admin (this app only)  │
 model-job   fetch-helper   serve-supervisor          ┌─────────────┴─────────────┐
 (untrusted  (sockets,      (forge-serve, proxy,      │ forge-modeld: scheduler   │
  files)      hashes)        rpc-serve; add-on)       │  ├ worker: model A        │
    │           │               │                     │  ├ worker: model B        │
    └── library dataset (verified, signed .fpk) ◄─────┤  └ library.db, trust store│
                                                      └───────────────────────────┘
 svc.keyring / SE04 (signing, model keys) · svc.audit · svc.power · svc.update (add-on)
```

### 6.2 Data model

Library database: `library.db` in the service's data directory on the verified dataset; `schema_version` 1; forward-only migration with a copy kept. App-side data (`~/.local/share/org.pbsd.model-manager/`): benchmark and evaluation results, UI state.

| Entity | Fields |
|---|---|
| `model` | `id` text (BLAKE3 of the signed digest, hex), `family_id` uuid, `display_name` text, `class` enum(text, embedding, stt, tts, ocr, vision, graph, plugin, adapter), `architecture` text, `template` text, `codec` text, `emb_codec` text, `params` i64, `context` i32, `file_path` text, `file_size` i64, `data_hash` blob(32), `format_version` i32, `encrypted` bool, `key_item` text?, `trust_state` enum(trusted, untrusted_key, unsigned, tampered, revoked), `quarantined` bool, `licence_spdx` text, `card_present` bool, `default_for_class` bool, `tags` text[], `note` text, `added_at` i64, `last_used_at` i64 |
| `signature` | `model_id`, `key_id` text(8), `public_key` blob(32), `valid` bool, `trusted` bool, `checked_at` i64 |
| `provenance` | `model_id`, `source_kind` enum(fpk, gguf, onnx, hf_dir, download), `source_hashes` json [{`path`, `blake3`, `sha256`, `size`}], `origin_url` text?, `import_options` json, `forge_version` text, `imported_by` text, `imported_at` i64, `parent_model_id` text? |
| `trust_key` | `key_id`, `public_key`, `name`, `roles` text[], `valid_from` i64?, `valid_until` i64?, `classes` text[]?, `revoked` bool (mirror of the FORGE-WP-33 trust store; that store is authoritative) |
| `policy` | `min_signers` i32, `key_sets` json (per class), `allow_user_self_signed` bool, `idle_unload_s` i32, `kv_spill` bool, `prefix_cache_per_app` i32, `battery_mode` enum(allow, defer_background, refuse) |
| `grant` | `app_id` text, `class` text, `mode` enum(deny, ask, any_trusted, named), `model_ids` text[], `max_concurrent` i32, `tokens_per_minute` i32, `max_context` i32, `priority` enum(interactive, foreground, background), `granted_at` i64 |
| `usage` | `app_id`, `model_id`, `day` date, `requests` i64, `prompt_tokens` i64, `completion_tokens` i64, `busy_ms` i64 |
| `adapter_link` | `base_model_id`, `adapter_model_id`, `name` text, `alpha` f32 |
| `job` | `id` uuid, `kind` enum(import, requantise, verify, bench, eval, download, dump, stats), `model_id`?, `params` json, `state` enum(queued, running, paused, done, failed, cancelled), `progress` f32, `started_at`, `ended_at`, `log_ref` text, `result_ref` text |
| `bench_run` | `id`, `model_id`, `device` text, `settings` json {`p`, `n`, `warmup`, `repeats`}, `prefill_tps_mean` f64, `prefill_tps_sd` f64, `decode_tps_mean` f64, `decode_tps_sd` f64, `peak_mem` i64, `forge_version`, `flags` text[] (battery, throttled, shared), `at` i64 |
| `eval_set` | `id`, `name`, `file_hash` blob(32), `items` i32, `scorer` text, `signed_by` text? |
| `eval_run` | `id`, `eval_set_id`, `model_id`, `seed` u64, `score` f64, `ci_low` f64, `ci_high` f64, `at` i64 |
| `endpoint` | `id`, `kind` enum(serve, proxy, rpc), `model_id`?, `socket` text, `args` json, `keep_running` bool, `state` enum(off, starting, up, failed) |
| `remote_device` | `id`, `address` text, `auth` enum(token, key), `keyring_item` text, `tls` enum(off, ca, fingerprint), `ca_ref` text?, `fingerprint` text?, `server_name` text? |

### 6.3 File formats and protocols

| Format or protocol | Read | Write | Library or own code | Parser runs in |
|---|---|---|---|---|
| `.fpk` version 1 | yes | yes | FORGE `modelfile` | model-job; model worker for loading |
| GGUF v2/v3 | yes | — | FORGE `import_gguf` | model-job |
| Hugging Face directory (`config.json`, safetensors, `tokenizer.json`) | yes | — | FORGE `import_hf` | model-job |
| ONNX (static-shape subset) | yes | — | FORGE `import_onnx` | model-job |
| Detached signature `.sig` (JSON, ≤ 64 KiB) | yes | yes | FORGE `integrity` | model-job |
| Ed25519 `.pub` (64 hex) | yes | yes | own | app host (fixed-length check) |
| Model card (Markdown), licence (text) | yes | yes | `markdownView` | app host, after size and encoding checks in model-job |
| Evaluation set (JSONL) | yes | — | own | model-job |
| Benchmark export (CSV, JSON, Markdown) | — | yes | own | app host |
| HTTPS download with ranges | yes | — | own HTTP/1.1 client on the platform network stack (FORGE-WP-21) | fetch-helper |
| Prometheus text from `/metrics` | yes | — | own line parser | serve-supervisor |
| OpenAI-style JSON from `/health`, `/v1/models` | yes | — | FORGE JSON | serve-supervisor |
| FORGE security policy JSON | — | yes | FORGE `security` | serve-supervisor, `forge-modeld` |
| Session bus records (`svc/model.ipc`, admin schema) | yes | yes | schema compiler (FORGE-WP-26) | app host |

No third-party library beyond what FORGE vendors (Monocypher). The Hugging Face importer and its tests use Python only at FORGE's build and test time, not at run time (`FORGE:README.md`).

### 6.4 Integration

- **Intents handled:** `open` and `view` for `application/vnd.forge.fpk`, `application/x-gguf`, `application/onnx` and `model://` URLs; `pick` with type `model` (other apps ask the user to choose a model; the reply is a model id, not a file).
- **Intents emitted:** `open` (Files, Passwords and Keys, Permissions, Software), `share` (benchmark report), `new` (key in SE04).
- **Services used:** `svc.model` and its admin interface, `svc.keyring`, `svc.settings`, `svc.notify`, `svc.audit`, `svc.power`, `svc.backup`, `svc.undo-trash`, `svc.update`, `svc.intent`, `svc.portal.file`.
- **Interface administered:** `svc.model.admin/1` — `list_models`, `register(handle)`, `remove`, `verify`, `set_policy`, `set_grant`, `revoke`, `loaded`, `unload`, `queue`, `set_budget`, `usage`, `restart`. Only this bundle id may call it, and mutating calls need a fresh administrator authentication token from SH03.
- **Other apps:** AI01, AI04, AX01, AX06, ME05, ME06, OF03, SH01 (model users); SE04 (keys); AD05 (grants view); AD04 (audit); SY05 (add-on install); SY04 (process and GPU graphs); SE02 (listener policy); AI03 (registers forecast and classifier bundles as class `plugin`).

Mapping of every FORGE command and option to this app (Verified options from `FORGE:docs/cli.md`, `FORGE:docs/serve.md`, `FORGE:docs/loadbalancer.md`, `FORGE:docs/rpc.md`):

| Command | Options | Where in the app |
|---|---|---|
| `forge info` | none | Hardware page (F074–F077) |
| `forge import` | `-o`, `--codec`, `--emb-codec`, `--template`, `--recipe`, `--output-tensor-type`, `--token-embedding-type`, `--tensor-type`, `--pure`, `--bf16-head`; env `FORGE_TEMPLATES` | Import wizard (F015–F026); templates directory fixed by the package |
| `forge inspect` | `--verify`, `--tensors`, `--blueprint` | Detail tabs Overview, Sections, Tensors, Blueprint (F035–F040) |
| `forge run` | `--prompt`, `--tokens`, `-n`, `--temp`, `--top-k`, `--top-p`, `--min-p`, `--repeat-penalty`, `--seed`, `--ctx`, `--ids`, `--no-bos`, `--logits`, `--verify`, `--grammar`, `--draft`, `--lookup`, `--ngram`, `--ngram-min`, `--spec-k`, `--codec`, `--policy`, `--input`, `--out` | Test run tab (F086–F091); `--verify` and `--policy` are always on and not shown; `--codec` belongs to import (no on-the-fly import of unsigned sources) |
| `forge bench` | `-p`, `-n`, `--warmup`, `--codec`, `--gemm` | Benchmarks (F092–F094) |
| `forge dump` | `<tensor> <out.f32>` | Tensors tab, "Export tensor" (F044) |
| `forge keygen` | `-o NAME` | Replaced by the SE04 hand-off (F051); key files importable (F052) |
| `forge sign` (model) | `--key`, `-o` | Sign dialog (F049, F050); `--key` replaced by `--signer` (FORGE-WP-33.3) |
| `forge sign` (detached) | `--key`, `-o`, `--name`, `--detached` | Sign detached (F060) |
| `forge verify` (model) | `--pubkey` (repeatable) | Verify action and Signatures tab (F040, F041); keys come from the trust store |
| `forge verify` (detached) | `--sig`, `--pubkey`, `--detached` | Detached signatures (F048) |
| `forge encrypt` | `-o`, `--key`, `--new-key` | Encrypt dialog (F058); key in the keyring instead of `K.mkey` |
| `forge security check` | `--policy`, `--json` | Policy check view (F061) |
| `forge rpc-serve` | `--port`, `--bind`, `--device`, `--token`, `--token-file`, `--authorized-keys`, `--pubkey`, `--tls-cert`, `--tls-key`, `--tls-client-ca`, `--tls-client-map`, `--quiet`, `--allow-scripts`, `--script-timeout`, `--script-mem-mb`, `--script-no-proc`, `--script-no-fs`, `--script-no-net`, `--policy` | Lend a device (F129–F131, F133, F134); bare `--token` is never used because command lines are visible to other users (`FORGE:docs/rpc.md:24`) |
| RPC client settings | `FORGE_RPC_TOKEN`, `_KEY`, `_TLS`, `_CA`, `_FINGERPRINT`, `_SERVER_NAME`, `_TLS_CERT`, `_TLS_KEY` | Remote devices (F132); secrets passed by descriptor, not environment (Proposed) |
| `forge script` | all | Out of scope here: DV01 and SY03. `--ask-model` is covered by apps using `svc.model` |
| `forge-serve` | `--host`, `--port`, `--max-active`, `--ctx`, `--codec`, `--watch`, `--no-pin`, `--numa-node`, `--mem-limit`, `--max-queue`, `--kv-spill`, `--policy` | Endpoint editor and dashboard (F121–F125); `--watch` is not offered because it needs `exec` under a policy (`FORGE:docs/serve.md:47`); `--codec` not offered (library models are already `.fpk`) |
| `forge-serve proxy` | `--backend`, `--route`, `--mode`, `--strategy`, `--host`, `--port`, `--retries`, `--health-ms`, `--policy` | Proxy editor and backend table (F126–F128) |
| Strict-loading and memory variables | `FORGE_REQUIRE_SIGNED`, `FORGE_TRUSTED_KEYS`, `FORGE_MODEL_KEY[_FILE]`, `FORGE_MEM_LIMIT`, `_FRACTION`, `_SOFT`, `_DEVICE_LIMIT`, `_PRESSURE`, `_SPILL_DIR`, `_SPILL_MAX`, `FORGE_DEVICE`, `FORGE_KV_CACHE_TYPE`, `FORGE_KV_OFFLOAD`, `FORGE_HUGEPAGES`, `FORGE_CACHE` | Trust policy (F055), Encrypt (F058), Budget editor (F078), KV spill policy (F116), Device choice (F080), KV codec (F082), Huge pages (F085), Import cache (F033) |

## 7. Security and privacy

### 7.1 Capabilities

| Capability | Why |
|---|---|
| `fs.app` | Benchmarks, evaluation results, UI state |
| `fs.portal` | Pick model files and folders to import; export models and reports; add a library location |
| `net.none` | Default |
| `net.user` | Download by URL only; held by `fetch-helper`; first contact per host prompted and audited |
| `net.route:clear`, `vpn`, `tor` | Route per download |
| `proc.helper:model-job`, `fetch-helper`, `serve-supervisor` | Section 6.1 |
| `ai.local`, `svc.model` | Test runs of trusted models and all administration |
| `svc.keyring` | Signing through SE04-held keys; model encryption keys; RPC tokens |
| `svc.settings`, `svc.notify`, `svc.audit`, `svc.power`, `svc.backup`, `svc.undo-trash`, `svc.update`, `svc.intent`, `svc.portal.file` | Section 6.4 |
| `bg.run` | Only when an endpoint has "Keep running"; off otherwise |
| `dev.gpu.compute` | Requested by the model worker, not the app; listed so the device test can ask (Phase 3) |

Not requested: `proc.exec`, `fs.path` (the library dataset is reached through the service, not by path), `bg.autostart`, any device portal. A listening socket has no capability in PLATFORM.md section 6; see section 13.

### 7.2 Threat model

Assets: the trust store and policy (they decide what code-like data every AI feature runs), model files, model encryption keys, signing keys (held by SE04), the grant table, usage records, served endpoints.

| Adversary | Attack | Mitigation |
|---|---|---|
| Malicious model file | Parser exploit in GGUF, safetensors, ONNX or `.fpk` readers | All parsing in `model-job` in capability mode with the source and output handles only; fuzzers exist in FORGE (`FORGE:fuzz/`) and are extended; quarantine until signed |
| Model with harmful behaviour | Backdoored weights that pass every hash | Signed-only policy with named keys and N-of-M (F055); provenance record; card and licence under the signature; no automatic downloads; evaluation harness to compare against a known-good file. A signature proves who vouched, not that the model is safe — stated in Help |
| Malicious download host | Serves a different file, or tracks the user | Checksum and signature before leaving quarantine; route choice; no catalogue calls; one prompt per host |
| Another app | Grants itself a model; reads another app's prompts through the cache or queue view | Admin interface bound to this bundle id plus administrator authentication; per-app prefix cache; the queue shows counts only; prompts are never logged (FORGE-WP-29.4) |
| Local unprivileged user | Changes policy or budget; replaces a model on disk | Library on the verified dataset; policy edits need authentication; tamper check on load and on schedule |
| Network peer | Uses an endpoint without permission; reads traffic | Add-on absent by default; Unix socket with peer credentials by default; TCP needs an explicit listen grant, and non-loopback shows a permanent warning because `forge-serve` has no authentication or TLS; `rpc-serve` never starts without tokens or keys; remote scripts locked off |
| Key thief | Steals a signing key or model key from disk or the environment | No key files (`forge keygen` and `.mkey` replaced by the keyring); keys passed by descriptor, never in argv or environment; imported key files offered for secure erase |
| Disk forensics | Recovers prompts from KV spill | Spill off by default; if on, only on an encrypted dataset, wiped at logout (F116) |
| Compromised app UI | Tricks the user into trusting a key | The authentication prompt is drawn by SH03 and names the key id and role |

Stored: section 6.2. Nothing about prompt or output content is stored anywhere by this app or the service. What leaves the machine: nothing by default; with a download, the HTTP request to the host the user typed; with a TCP endpoint or lent device, whatever clients request.

## 8. Settings

| Key | Type | Default | Description |
|---|---|---|---|
| `library.view` | enum(list, grid) | list | Library layout |
| `library.group_families` | bool | true | Group quantisations of one source |
| `library.extra_locations` | list of portal tokens | empty | Additional library locations |
| `import.default_codec` | string | `q4_k_m` | Preselected codec for Hugging Face sources |
| `import.default_recipe` | enum(forge, llama) | forge | Mixed-precision recipe |
| `import.keep_source` | bool | true | Keep the source after a successful import |
| `import.require_card` | bool | true | Refuse to finish without a card and a licence |
| `verify.on_load` | bool | true | Hash check when the service loads a model (always on in the app profile; shown read-only) |
| `verify.schedule_days` | int | 7 | Background re-verification interval; 0 = off |
| `bench.prompt_tokens`, `bench.new_tokens`, `bench.warmup`, `bench.repeats` | int | 32, 32, 1, 5 | Benchmark defaults |
| `bench.exclude_flagged` | bool | true | Leave flagged runs out of comparisons |
| `download.enabled` | bool | false | Show the Download action |
| `download.route` | enum(clear, vpn, tor) | clear | Default route |
| `download.rate_limit_kbps` | int | 0 | 0 = unlimited |
| `jobs.max_parallel` | int | 1 | Heavy jobs at once |
| `service.idle_unload_s` | int | 600 | Mirror of `policy.idle_unload_s` |
| `service.battery_mode` | enum(allow, defer_background, refuse) | defer_background | Mirror of the policy field |
| `serving.show` | bool | false | Show the Serving section when the add-on is absent |
| `ui.show_command_preview` | bool | true | Show equivalent `forge` commands |

Policy, budgets, grants and trust are not app settings; they live in the service and need authentication.

## 9. Accessibility and languages

- Trust state is always a word plus an icon; the a11y name of a model row reads "qwen3-8b, q4_k_m, 5.1 gigabytes, signed and trusted, loaded".
- The permission matrix is a grid with row and column headers; each cell's name is "Assistant, text generation, any trusted model".
- Hashes and key ids are in the monospace face, grouped in fours, and are read in groups by the screen reader.
- Charts have a table alternative (`Ctrl+Alt+T`).
- Codec names and command lines are not translated; their descriptions are.
- Job progress is a live region that speaks at 25% steps and on completion.

## 10. Performance budgets

| Measure | Budget |
|---|---|
| Cold start to first frame | under 400 ms |
| Library of 500 models listed | under 100 ms |
| Inspect overview of a 40 GB `.fpk` | under 300 ms (reads the header, table and manifest only) |
| Hash verification | disk-bound; progress every 250 ms; UI stays at 60 frames per second |
| Import | no faster or slower than `forge import`; job overhead under 2% |
| App memory | under 120 MB; `model-job` peak is one tensor for streamed imports (`stream_writer`, `FORGE:docs/modelfile.md`) |
| Dashboard refresh | 1 Hz scrape, under 1 ms UI cost per update; zero wake-ups when the page is hidden |
| Permission change to enforcement | under 100 ms |

## 11. Testing and proof

- **Unit:** trust-policy evaluation (N-of-M, per-class sets, revocation), grant resolution, fit estimates, command preview strings against `forge` argument parsing, benchmark statistics.
- **Golden renders:** every page and dialog in 5.2 and 5.3, three themes, 200% scale, narrow.
- **Scripted UI:** tasks T1–T15 against a fake service and FORGE's tiny golden models (`golden/tiny_f32.fpk`, built by FORGE's ctest).
- **Differential:** each UI action against the matching `forge` CLI result on the same input (import bytes, inspect listing, verify verdict, signature bytes).
- **Fuzz:** GGUF, safetensors, ONNX and `.fpk` readers in `model-job` (extending `FORGE:fuzz/`), `.sig` JSON, the Prometheus parser, download range logic, evaluation JSONL.
- **Negative:** unsigned, wrong-key, revoked-key and tampered files are never grantable; an app that is not this bundle cannot call the admin interface; a TCP endpoint cannot start without the listen grant.
- **Fault:** kill `model-job` mid-import (no partial file in the library), kill a model worker (service survives), pull the library disk.

| Module | Property | Target tier | How it would be checked | What blocks a higher tier |
|---|---|---|---|---|
| Trust decision | accept ⇒ enough non-revoked keys with role `model` signed this exact file | T1 | Shared proof with FORGE-WP-33 decision function | Needs the trust store format frozen |
| Grant resolution | An app without a matching grant never receives a session; revoke cancels | T2 | PRISM on the admission state machine (FORGE-WP-29) | Concurrency model of the scheduler |
| Quarantine rule | No quarantined or tampered model is reachable through `svc.model` | T2 | PRISM invariant on the library state machine | — |
| `.fpk` header and table reader | No out-of-bounds read on any input | T2 | Absence of runtime errors | Rest of `modelfile` stays T3 |
| Importers (GGUF, HF, ONNX) | Memory safety | T3 | Safe subset, sanitisers, fuzzing | Format size and variety |
| Download resume | Final file hash equals the hash of a single-pass download | T3 | Property tests with injected disconnects | Network stack is separate |
| Serving supervisor | Never starts a listener without a recorded grant | T2 | PRISM on the launcher | — |
| Numerics (kernels, codecs) | — | T3 | FORGE oracle tests (`FORGE:docs/testing.md`) | Floating point |
| GPU paths | — | T4 | Not assessed until FORGE-WP-32.2 | Drivers |

Nothing above has been run.

## 12. Build plan

### Phase 1 — library, trust, permissions

| ID | Work item | Depends on |
|---|---|---|
| AI02-W01 | Admin schema `svc.model.admin/1` and library record format, agreed with FORGE-WP-29.3 | G-08, FORGE-WP-26, FORGE-WP-29 |
| AI02-W02 | Main window, library table, inspector, filters | G-01, G-02, FORGE-WP-13 |
| AI02-W03 | `model-job` helper: inspect, verify, import (`.fpk`, GGUF, Hugging Face) as jobs | G-06, G-07, FORGE-WP-27, FORGE-WP-28 |
| AI02-W04 | Import wizard with codec, template, card and licence; quarantine | W02, W03 |
| AI02-W05 | Trust store view, trust-a-key, policy editor, signing through the keyring | FORGE-WP-33, SE04, SH03 |
| AI02-W06 | Permissions matrix, first-use prompt wiring, revoke | W01, SH04, AD05 |
| AI02-W07 | Loaded models, queue, idle unload, service health | W01 |
| AI02-W08 | Hardware page and budget editor | W01, `svc.power` |
| AI02-W09 | Test run tab and benchmark with history | W03 |
| AI02-W10 | Disk page, clean up, jobs page, notifications, first-run page | W03 |

### Phase 2 — conversion, download, comparison, serving

| ID | Work item | Depends on |
|---|---|---|
| AI02-W11 | Mixed-precision options, re-quantise, batch conversion, ONNX import | W04 |
| AI02-W12 | Download helper: URL, checksum, resume, route, badge | FORGE-WP-21, SE02 |
| AI02-W13 | Benchmark comparison, flags, export | W09, AI02-GAP-2 |
| AI02-W14 | Quotas, priorities, usage history, prefix-cache and spill policy | W06, FORGE-WP-29.2, FORGE-WP-29.5 |
| AI02-W15 | Revocation handling; tamper state and scheduled re-verification | FORGE-WP-33.2 |
| AI02-W16 | Encrypt with keyring-held keys; key-by-descriptor load path | FORGE change to `FORGE_MODEL_KEY` handling (Proposed) |
| AI02-W17 | Serving add-on: supervisor, endpoint editor, Unix socket, dashboard | FORGE-WP-32.1, SY05, listen capability (section 13) |
| AI02-W18 | Detached signatures, policy check view, command preview, live pools | W05 |
| AI02-W19 | Tags, families, duplicates, grid view, export model | W02 |

### Phase 3 — evaluation, adapters, remote compute

| ID | Work item | Depends on |
|---|---|---|
| AI02-W20 | Evaluation sets, scorers, runs, quantisation drift | W09 |
| AI02-W21 | Adapter import converter, attach, compatibility check | FORGE adapter converter (Proposed) |
| AI02-W22 | Proxy editor and backend table | W17 |
| AI02-W23 | Lend a device and remote devices, with secrets by descriptor | W17, FORGE change to `FORGE_RPC_*` handling (Proposed) |
| AI02-W24 | Device test, device choice, KV codec, NUMA, huge pages | FORGE-WP-32.2, FORGE-WP-32.3 |
| AI02-W25 | Graph view, tensor statistics, file compare, dump | W03 |
| AI02-W26 | Model classes beyond text in the library and permissions UI | FORGE-WP-31 |
| AI02-W27 | Snapshot restore hand-off; removable library locations | AD03 |

## 13. Open questions and risks

1. **Who packages `forge-modeld`.** The daemon is FORGE work (FORGE-WP-29). This spec assumes it ships as a system service package and that this bundle is its only administrator. The manifest lists `svc.model` under `services_provided` to record that ownership; if the platform wants services declared elsewhere, that line moves.
2. **No listen capability.** PLATFORM.md section 6 has `net.host`, `net.user` and `net.route` but nothing for accepting connections. Serving needs `net.listen:<address>:<port>` and a Unix-socket variant. New gap.
3. **No administrator-authentication primitive** is named in PLATFORM.md. Policy, budget and trust edits need one (assumed from SH03).
4. **Keys in files and environment.** FORGE reads signing keys from files and model keys and RPC secrets from environment variables today. FORGE-WP-33.3 covers signing; the model-key and RPC paths need the same treatment (Proposed, not yet in the FORGE build plan).
5. **GPU.** Every GPU row on the Hardware page may read "not found" on HardenedBSD for a long time (FORGE build plan Q13). The app must be complete on CPU.
6. **Importers are tested on small random models only**, not full-size published checkpoints (`FORGE:docs/modelfile.md`). Real imports may fail; errors must be shown verbatim and jobs must fail cleanly.
7. **Which models ship** and under which licences is undecided (Q11). The first-run page cannot point to a bundled model yet.
8. **Proxy streaming.** The proxy buffers responses; a served chat behind it does not stream. Fixing that is FORGE work.
9. **Signature meaning.** Users may read "signed" as "safe". Wording and Help must keep the two apart.
10. **Benchmark honesty.** Thermal and background load change results; flags reduce but do not remove the problem.
11. **MIME types** for `.fpk`, GGUF and ONNX are not registered anywhere; the names used here are placeholders for the platform MIME registry.
