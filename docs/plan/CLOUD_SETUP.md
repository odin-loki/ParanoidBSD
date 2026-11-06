# PBSD cloud burst: setup guide

This is the step-by-step guide for running PBSD with GLM-5.3-Flash through the API, with the gates (builds, proof checks, QEMU tests) spread over Odin's P920 and rented cloud cores.

- The plan is in `AGENTS.md`.
- The runner is `tools/runner` (C++), the gate scripts are in `tools/gates`, and the cloud and PC setup scripts are in `tools/cloud`.

**About the figures:**
- Every price below was read from the provider's own price feed or page on 4 Nov 2026 (AEDT). Spot prices move daily, so check them again on the day you rent.
- Conversions use the RBA rate of 3 Nov 2026: **US$1 = A$1.4335** ([RBA](https://www.rba.gov.au/statistics/frequency/exchange-rates.html)).
- Run times and token costs are estimates from `pbsd_run estimate` (AGENTS.md section 8.1), not measurements, until the step 10 pilot.

## 1. What to rent and why

The bottleneck is **gate CPU**: about 30,000 core-hours of builds, proof checks and QEMU in the middle case (17,900–59,600; AGENTS.md section 8.1). The API is not the limit, so the money goes on **CPU cores, not GPUs**. The PC's own RTX 3090 runs the Lean prover.

### Recommended setup

| Role | Machine | Cores | Why |
|---|---|---|---|
| Runner, git checkpoints, 19 local gate slots (compile, proof, and **all QEMU gates**) | Odin's P920 (WSL2, nested virtualization on for KVM) | 28 free | already owned; the only machine here with KVM |
| Gate worker **w1** + git mirror | **1 × GCP `c3d-highcpu-360` Spot, `australia-southeast1` (Sydney)** | 180 physical (360 vCPU), 720 GB RAM | cheapest per core found in Sydney, a few ms from the PC. **No KVM** (GCP AMD machines have no nested virtualization), so it takes compile and proof gates only |
| Fallback, or a second worker with KVM | AWS `m7a.metal-48xl` Spot, `eu-north-1` (or `c7a.metal-48xl`) | 192 physical, 768 / 384 GB, KVM | cheaper per core than GCP Sydney. Overseas, which adds ~300 ms per gate push, negligible against a multi-minute gate. Metal is needed for KVM on AMD |

**Why one big box:** it is simpler than several small ones. One bootstrap, one mirror, one multiplexed ssh connection, and one thing to delete at the end. 720 GB of RAM lets the whole worker run in tmpfs.

**Run length:** w1 (88 gate slots) plus the PC (19 slots) give about 208 cores against about 30,000 core-hours. The gate work then takes about **6 days** of wall clock (3.6–12 days). The PC alone would take about 44 days.

**Agents:** set `slots.agent.count` to about 224 (already done in `config.glm-api-burst.json`). Above about 200, agents only wait for gate slots.

### CPU prices found (USD per hour unless marked)

| Provider / machine | Cores | Price | US$/core-hour | KVM | Source |
|---|---|---|---|---|---|
| **GCP c3d-highcpu-360 Spot, Sydney** | 180 (360 vCPU) | **$1.59984/h** (on-demand $16.86645) | 0.0089 | no | [Google Spot pricing](https://cloud.google.com/spot-vms/pricing) |
| GCP c3d-highcpu-360 Spot, Iowa/Oregon/S. Carolina | 180 | $3.0924/h | 0.0172 | no | same |
| AWS c7a.48xlarge Spot, us-west-1 | 192 (no SMT) | $1.2287/h | 0.0064 | no (metal only) | [AWS spot feed](https://aws.amazon.com/ec2/spot/pricing/) |
| AWS c7a.48xlarge Spot, eu-north-1 | 192 | $1.5306/h | 0.0080 | no | same |
| **AWS c7a.metal-48xl Spot, eu-north-1** | 192 | **$1.0548/h** | **0.0055** | **yes** | same |
| **AWS m7a.metal-48xl Spot, eu-north-1** (interruption band under 5%) | 192 | **$1.1823/h** | **0.0062** | **yes** | same |
| AWS c8i.48xlarge Spot, Sydney (Intel, nested virtualization) | 96 (192 vCPU) | $1.1748/h | 0.0122 | yes | same |
| AWS c7a in Sydney | – | not offered in ap-southeast-2 | – | | same |
| Hetzner AX162-1 dedicated (EPYC 9454P) | 48 (96 threads) | €1.171/h incl. 19% VAT (€0.984 net, cap €614/month) **+ €361.76 setup** | ≈0.02 + setup | yes | [Hetzner](https://www.hetzner.com/dedicated-rootserver/matrix-ax/) |
| Hetzner Cloud CCX63 | 24 (48 vCPU) | €1.3686/h net | ≈0.064 | | [Hetzner price list](https://docs.hetzner.com/general/infrastructure-and-availability/price-adjustment/) |
| OVH | – | monthly dedicated only (setup fee equal to a month without commitment) | not hourly | | [OVH](https://www.ovhcloud.com/en-au/bare-metal/prices/) |

- **Hetzner:** the June 2026 price rise and setup fees make it poor value for a run of under a week.
- **Vultr:** no VM of 96+ vCPU is listed in Sydney today.
- **Spot caveat:** spot machines can be reclaimed at any time. The runner treats a lost worker as an infrastructure error (ssh exit 255): the gate is retried elsewhere, no attempt is charged, and no work is lost, because all commits live on the PC. Keep the PC's own 19 gate slots running throughout.
- **Quotas:** new AWS accounts start with a spot quota of 5 vCPUs per region. Ask early (step 2).

### GPU: does renting one help?

**No, not for the bottleneck.** The gates are compilers, proof checkers (PRISM with ESBMC and CBMC, Lean, Apalache) and QEMU, and none of them use a GPU.

**The one GPU job is the Lean prover,** the Goedel model on FORGE inside `prism prove`, on the PC's RTX 3090. Rent a GPU only if you want it off the PC. Prices found (Sep 2026 snapshot, [hostalu](https://hostalu.com/en/gpu-cloud/), [RunPod](https://www.runpod.io/pricing)):

| GPU | RunPod community / secure | Vast.ai median | Lambda |
|---|---|---|---|
| RTX 4090 24 GB | $0.34 / $0.74 | $0.50 | – |
| RTX A6000 48 GB | $0.33 / $0.53–0.59 | $0.47 | $1.09 |
| A100 80 GB | $1.19–1.39 / $1.59 | $0.83 (SXM) | – |
| H100 80 GB | $1.99–2.69 / $2.89–3.49 | $2.77 | $3.29–3.99 |

### Budget

| Item | Expected | Range | Source |
|---|---|---|---|
| GLM-5.3-Flash API (Morph direct, fp8) | US$1,582 | $774–3,747 (up to $6,044 if Morph's page prices apply) | `pbsd_run estimate`, AGENTS.md sections 7.3 and 8.1 |
| w1: GCP c3d-highcpu-360 Spot Sydney, about 6 days, 87% of the gate work | US$287 | $172–574 | Google spot price; includes 25% for idle and boot time |
| w1 boot disk (100 GB pd-balanced) and egress | under US$15 | | small; gate traffic is a few GB |
| Pilot (step 10), 447 tasks | US$143 of API | $70–338 | part of the run, not extra: pilot work is kept |
| PC electricity, about 8 days at 450 W | US$19 | | A$0.33/kWh |
| GPU | US$0 | | the Lean prover runs on the PC's RTX 3090 |
| **Total expected** | **≈ US$1,900 (A$2,720)** | | |
| **Budget to set aside (high case + 15%)** | **US$5,000 (A$7,200)** | | |

Set billing alerts at A$300 and A$800 on the cloud account and a hard spend limit on every API key (step 3).

## 2. Step by step

You (Odin) do steps 1–4 and the renting yourself; nothing here signs up or rents anything for you.

1. **Create accounts.**
   - Google Cloud (console.cloud.google.com): create a project `pbsd`, add a billing account (AUD card).
   - Morph (morphllm.com/dashboard): the only model provider (AGENTS.md section 7.3).
     - **First, check that you can top up:** add a small amount of credit, and only then go further.
     - Then create an API key.
   - Optional, for a KVM worker or a GCP fallback: AWS.
2. **Ask for capacity now; these take time.**
   - **Morph:** in the dashboard, check the price of `morph-glm53flash`. Its model list says $0.047 / $0.02 / $0.268 per 1M; its marketing page says $0.15 / $0.01 / $0.42.
     - Check your account's rate limit too.
     - The burst run needs about 90 requests in flight, so ask Morph for about 100 if it is lower. The pilot needs far fewer, so it can start before the answer arrives.
   - **GCP quotas:** IAM & Admin → Quotas, region `australia-southeast1`. Raise "C3D CPUs" and "Preemptible CPUs" to at least 400. New projects start far below that, and the machine will not start until the request is approved. Also request the same in `us-central1` as a backup.
   - **AWS (if used):** Service Quotas → EC2 → "All Standard Spot Instance Requests" to 400 vCPUs in `eu-north-1` (the default is 5).
3. **Make the API key and set a spend limit.**
   - **Morph:** add about US$200 of credit for the pilot, and more after it. Set a spending limit if the dashboard offers one; otherwise keep the credit balance as the limit.
   - **Config:** if the dashboard's prices differ, put them into both configs (`providers[0]` in `config.glm-api*.json`). The shipped values are `morph-glm53flash` and $0.047 / $0.02 / $0.268.

   Keep the key in your password manager.
4. **Put the keys in WSL2 on the PC** (not in the repo). In `~/.pbsd/env` (chmod 600):
   ```bash
   export MORPH_API_KEY=...          # Morph, the only model provider (AGENTS.md 7.3)
   export PRISM_BIN=$HOME/PRISM/build/prism PRISM_PROVER_FPK=/mnt/d/forgebox/models/goedel-8b-q4_k.fpk PRISM_DEVICE=cuda   # PRISM and the Lean prover on FORGE; the 32B is /home/odin/models/goedel32.fpk (slower, same result on easy lemmas)
   export PBSD_STATE=/run/pbsd/state PBSD_WT=/run/pbsd/wt PBSD_REPO=/run/pbsd/repo PBSD_CHECKPOINT=/mnt/d/pbsd-ckpt
   export PBSD_CONFIG=$HOME/pbsd/docs/plan/tools/runner/config.glm-api-burst.json
   ```
   Add `. ~/.pbsd/env` to `~/.bashrc`. The gates never see these variables, because the runner scrubs the gate environment.

   **Check that Morph answers.** The command below should print a short reply.
   ```bash
   curl -s https://api.morphllm.com/v1/chat/completions -H "Authorization: Bearer $MORPH_API_KEY" -H 'Content-Type: application/json' \
     -d '{"model":"morph-glm53flash","max_tokens":200,"messages":[{"role":"user","content":"Say ok"}]}' \
     | jq -r '.choices[0].message.content // .error.message // .message, (.usage | tostring)'
   ```
   The second line shows the `usage` block. Check whether it has `prompt_tokens_details.cached_tokens`, which the runner uses to bill cache hits.
5. **Make the runner's ssh key** (WSL2): `ssh-keygen -t ed25519 -f ~/.ssh/pbsd_runner -N ''`. Paste `~/.ssh/pbsd_runner.pub` into `tools/cloud/cloud-init.yaml` where it says REPLACE, and keep that edited copy outside the repo.
6. **Toolchains, proof tools, pinned gates and reference trees on the PC.**
   ```bash
   sudo LINUX_TAG=<newest longterm tag on kernel.org> WINE_TAG=<current Wine stable tag> PBSD_USER=$USER \
        bash docs/plan/tools/cloud/pc-tools.sh
   ```
   It installs the build toolchain, QEMU for three architectures, Apalache and Lean (elan). ESBMC, CBMC and cppcheck are built from PRISM's pinned sources by `tools/cloud/build-prism.sh` (next bullet), not installed here. It also copies the **pinned** gate scripts to `/opt/pbsd/gates` (`gates_dir`) and the task list to `/opt/pbsd/tasks.json`, and clones Linux and Wine at the tags you give under `~/pbsd-ref` (`ref_roots`).
   - **Tags:** write the two tags down. Every worker must use the same ones, because they are the behavioural reference for every port.
   - **KVM:** for QEMU gates with KVM under WSL2, set `nestedVirtualization=true` in `%UserProfile%\.wslconfig`.
   - **Gate hooks:** set the real build commands once the P0-07 tasks have passed: `PBSD_BUILD_CMD`, `PBSD_TEST_CMD`, `PBSD_QEMU_IMAGE` and `PBSD_QEMU_MARKER`. The proof hook defaults to `proof_check.sh`, with the tier taken from the task.
   - **Exit codes:** each gate exits 0 pass, 1 fail, 3 infrastructure error.
   - **Lean and Mathlib for the prover:** `tools/cloud/lean-mathlib.sh` (Lean 4.9.0 + Mathlib v4.9.0, the version Goedel was built for). Without Mathlib the model's proofs fail as "unknown tactic".
   - **PRISM and FORGE:** build PRISM and its analyzers with `tools/cloud/build-prism.sh` (clones `~/PRISM`, fetches PRISM's pinned clang 23.1.4 because clang 18 cannot build it, builds PRISM with FORGE, then builds cppcheck, CBMC and ESBMC from the commits PRISM pins; about 20 minutes on 64 cores). Put your Goedel `.fpk` somewhere and set `PRISM_PROVER_FPK` to it. `prism-deps tool` only fetches and hash-checks analyzer source; `build-prism.sh` compiles it into `~/.prism/tools`. PRISM looks in `--tool`, then `$PRISM_TOOLS_DIR` / `~/.prism/tools`, then `PATH`. Check: `prism --version`, `ls ~/.prism/tools`, then `prism prove` on one small `sorry` lemma.
7. **Rent w1** (only when step 2's quota is approved and you are ready to run).
   ```bash
   gcloud compute instances create pbsd-w1 --project pbsd --zone australia-southeast1-b \
     --machine-type c3d-highcpu-360 --provisioning-model SPOT --instance-termination-action DELETE \
     --image-family ubuntu-2404-lts-amd64 --image-project ubuntu-os-cloud --boot-disk-size 100GB \
     --labels pbsd=1 --metadata-from-file user-data=$HOME/.pbsd/cloud-init.yaml
   gcloud compute firewall-rules create pbsd-wg --allow udp:51820 --target-tags pbsd   # only if using WireGuard
   ```
   Bootstrap runs by itself via cloud-init (log: `/var/log/pbsd-bootstrap.log`). It:
   - installs the toolchain, QEMU, Apalache and Lean (ESBMC, CBMC and cppcheck arrive with PRISM in step 9);
   - creates user `pbsd` with your key;
   - mounts a tmpfs at `/run/pbsd` (70% of RAM);
   - creates the bare mirror `/srv/pbsd/pbsd.git`;
   - clones the Linux and Wine reference trees;
   - installs the pinned gates in `/opt/pbsd/gates` and the entry point `/opt/pbsd/bin/pbsd-gate`;
   - lets gates run in a network namespace without network, and blocks the metadata service for `pbsd`;
   - prints which tools are present.

   A missing tool makes its gates exit 3 (retried), never pass. Without cloud-init: `scp -r docs/plan/tools docs/plan/tasks.json root@w1:/root/pbsd-plan/`, then `sudo PBSD_PUBKEY="$(cat ~/.ssh/pbsd_runner.pub)" LINUX_TAG=... WINE_TAG=... bash /root/pbsd-plan/tools/cloud/bootstrap.sh`.
8. **Connect the PC to w1.** Plain ssh with key-only login is enough. WireGuard is optional: it gives a private 10.66.0.0/24 network, so nothing on your home router needs opening, because the PC dials out.
   - In WSL2: `sudo apt install wireguard-tools`, then fill in `tools/cloud/wg-pc.conf.example` as `/etc/wireguard/wg0.conf` and run `sudo wg-quick up wg0`. Put `wg-worker.conf.example` on w1 the same way. WSL2 kernels since 5.6 include WireGuard; if yours does not, use the Windows WireGuard app with the same file.
   - Append `tools/cloud/ssh_config.example` to `~/.ssh/config`. It defines the aliases `w1` and `pbsd-mirror`, with connection multiplexing so ~90 gates share one connection. Test with `ssh w1 nproc`.
9. **Seed the mirror and register the worker.**
   ```bash
   PRISM_BIN=$PRISM_BIN tools/cloud/mirror-setup.sh $PBSD_REPO pbsd-mirror <base-branch>   # pushes the full history once, builds w1's hbsd/kde reference trees, copies PRISM and ~/.prism/tools to w1
   tools/cloud/register.sh w1                                         # adds "w1 88" to ~/.pbsd/workers.txt ("kvm" is added only where /dev/kvm exists)
   ```
   - **Updates:** the runner re-reads `workers.txt` every 30 s, so workers can be added or removed while it runs.
   - **What happens per gate on a worker:**
     1. The runner commits the task's owned files with a `PBSD-Base` trailer.
     2. It pushes the commit to the mirror as `refs/gates/<task>`.
     3. It runs `ssh w1 -- /opt/pbsd/bin/pbsd-gate <role> <task> <sha> <proof-tier>`.
     4. That checks the commit out in tmpfs with the same sparse pattern as the PC's worktrees, and runs the pinned `/opt/pbsd/gates/<role>.sh`.
   - **QEMU gates** never go to w1, because it has no KVM.
10. **Pilot: 447 tasks, go/no-go.** Measure before spending the big money. The pilot runs in the real database, so its work is kept for the full run.
    ```bash
    # All of Phase 0 with every package's -IF task, P0-FREEZE and all of P1-02: 447 tasks, closed under their
    # dependencies, including three package models (Lean prover) and 29 T2 proofs.
    jq '[.[] | select((.wp|test("^P0|^P1-02")) or (.id|endswith("-IF")))]' docs/plan/tasks.json > ~/.pbsd/tasks.pilot.json
    PBSD_TASKS=~/.pbsd/tasks.pilot.json pbsd_run init
    PBSD_TASKS=~/.pbsd/tasks.pilot.json pbsd_run run       # inside tmux
    pbsd_run status
    sqlite3 $PBSD_STATE/tasks.db "select t.role, json_extract(t.body,'$.proof_tier'), count(distinct c.task), round(sum(c.usd),2) from cost c join task t on t.id=c.task group by 1,2"
    ```
    **Check:**
    - gates pass on w1 as well as on the PC;
    - real core-minutes per gate;
    - tokens and US$ per task class;
    - **T1/T2 obligations proved per attempt**;
    - review rejection rate;
    - Morph's 429 rate. If it throttles, lower `concurrency` in the config or ask Morph for more.

    Then rerun `pbsd_run estimate cores=208` with the measured numbers, for example `implementer.attempts=1.9 T2.attempts=5 hit=0.8`. **Stop and decide** if:
    - T2 proofs stay under about 50% after 3 attempts;
    - the estimate is over budget;
    - review rejections exceed 30%.

    The options are in AGENTS.md section 8.5, step 4.
11. **Full run.** `pbsd_run run` inside `tmux` (it loads the full task list, keeping the passed pilot tasks), so it survives the terminal closing. It resumes from the tmpfs database or the HDD checkpoint after any crash or reboot.
12. **Monitor.** `pbsd_run status` (or the live screen) shows:
    - overall and per-phase progress bars, time left and finish time;
    - cost so far and projected;
    - gate slots (`compile 8/8 qemu 2/2 proof 9/9 workers 1 (85/88 slots)`);
    - one line per provider.

    Check twice a day:
    - GCP console → Billing → Reports; Z.ai and fallback usage pages.
    - **If w1 is preempted:** `gcloud compute instances list --filter=labels.pbsd=1`. Recreate it with the step 7 command (same name), then rerun step 9's `mirror-setup.sh` and `register.sh`. The tasks that were gating there were already retried elsewhere, with no attempt charged.
    - **If Spot in Sydney keeps getting reclaimed:** rent the AWS fallback (step 7 with the AWS CLI, or the console; same cloud-init file) and register it as `w2`. Metal types register with `kvm`, so they also take QEMU gates.
    - **Escalated tasks:** `sqlite3 $PBSD_STATE/tasks.db "select id from task where state='escalated'"`. Their messages and last gate logs are in the `message` and `event` tables.
13. **Stop.** Ctrl-C the runner (it checkpoints on exit), or let it finish.
14. **Tear down so billing stops.** The same day the run ends:
    ```bash
    tools/cloud/teardown.sh                                         # drains workers, lists what still bills
    gcloud compute instances delete pbsd-w1 --zone australia-southeast1-b --quiet
    gcloud compute disks list --filter=name~pbsd                    # delete anything left
    aws ec2 terminate-instances --instance-ids <id>                 # only if you used AWS
    ```
    Nothing on w1 needs saving: every commit lives on the PC and its HDD checkpoint. Then revoke or lower the API key spend caps.

## 3. Cost checklist

- [ ] Billing alert on GCP at A$300 and A$800; spend cap on every API key.
- [ ] Spot price for `c3d-highcpu-360` in `australia-southeast1` checked on the day (expect about US$1.60/h).
- [ ] `pc-tools.sh` run; the Linux and Wine tags written down and passed to every worker.
- [ ] PRISM built; `prism --version` works; `PRISM_PROVER_FPK` set; `prism prove` closed one small lemma; `proof_check.sh` calibrated by hand on one unit (AGENTS.md section 10).
- [ ] `pbsd_run selftest`: 23 checks ok.
- [ ] Pilot (step 10) done and the estimate re-run with measured numbers; go/no-go decided.
- [ ] Only one big worker running, unless the pilot says the PC + w1 cannot keep the agents busy.
- [ ] After the run: `teardown.sh` shows nothing, the instance and its disk are deleted, and the next day's billing report shows no compute charges.
