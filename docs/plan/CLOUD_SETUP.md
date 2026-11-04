# PBSD cloud burst: setup guide

This is the step-by-step guide for running PBSD with GLM-5.3-Flash through the API, with the gates
(builds, PRISM proofs, QEMU tests) spread over Odin's P920 and rented cloud cores. The plan is in
`AGENTS.md`; the runner is `tools/runner` (C++), the gate scripts are in `tools/gates` and the
cloud scripts are in `tools/cloud`.

Every price below was read from the linked page on 4 Nov 2026 (AEDT). Spot prices move daily, so check
them again on the day you rent. Conversions use the RBA rate of 3 Nov 2026: **US$1 = A$1.43**
([RBA](https://www.rba.gov.au/statistics/frequency/exchange-rates.html)). Run times and token costs are
estimates from `pbsd_run estimate`, not measurements, until the step 10 calibration run.

## 1. What to rent and why

The bottleneck is **gate CPU**: about 10,000 core-hours of builds, PRISM and QEMU for 16,853 task
attempts (12,964 tasks × 1.3 for splits and retries). The API is not the limit: Z.ai alone covers
the request rate. So the money goes on **CPU cores, not GPUs**.

### Recommended setup

| Role | Machine | Cores | Why |
|---|---|---|---|
| Runner, git checkpoints, 19 local gate slots | Odin's P920 (WSL2) | 28 free | already owned |
| Gate worker **w1** + git mirror | **1 × GCP `c3d-highcpu-360` Spot, `australia-southeast1` (Sydney)** | 180 physical (360 vCPU), 708 GiB RAM | cheapest per core found today, and it is in Sydney (a few ms from the PC) |
| Fallback if GCP Spot is unavailable or preempted for long | 1 × AWS `c7a.48xlarge` Spot, `us-west-1` or `eu-north-1` | 192 physical (no SMT), 384 GiB | about the same price, but overseas (adds ~150–250 ms per gate push, which is negligible against a multi-minute gate) |

One big box is simpler than several small ones: one bootstrap, one mirror, one ssh connection
(multiplexed) and one thing to delete at the end. 708 GiB of RAM lets the whole worker run in tmpfs.

With w1 (88 gate slots) plus the PC (19), the run has about 208 cores against 10,000 core-hours,
so the gate work takes about **2.2–2.7 days** of wall clock (the plan's 128-core figure was 2.7 days;
180 cores gets a little under that). Set `slots.agent.count` to about 144 (already done in
`config.glm-api-burst.json`); above that, agents only wait for gate slots.

### CPU prices found (USD per hour unless marked)

| Provider / machine | Cores | Price | US$/core-hour | Source |
|---|---|---|---|---|
| **GCP c3d-highcpu-360 Spot, Sydney** | 180 (360 vCPU) | **$1.518/h** (on-demand $16.81) | 0.0084 | [Usage AI](https://instances.usage.ai/gcp/compute/c3d-highcpu-360) |
| GCP c3d-highcpu-360 Spot, Iowa/Oregon | 180 | $3.09–3.25/h | 0.018 | [Google Spot pricing](https://cloud.google.com/spot-vms/pricing), [Usage AI](https://instances.usage.ai/gcp/compute/c3d-highcpu-360) |
| AWS c7a.48xlarge Spot, us-west-1 | 192 (no SMT) | $1.23/h | 0.0064 | [aws-pricing.com](https://aws-pricing.com/c7a.48xlarge.html), [Usage AI](https://instances.usage.ai/aws/ec2/c7a.48xlarge) |
| AWS c7a.48xlarge Spot, eu-north-1 / Singapore | 192 | $1.25–1.41 / $1.91 | 0.007–0.010 | same |
| AWS c7a in Sydney | – | not offered in ap-southeast-2 | – | same |
| AWS c7i.48xlarge Spot, Sydney | 96 (192 vCPU) | $4.22/h | 0.044 | [Vantage](https://instances.vantage.sh/aws/ec2/c7i.48xlarge) |
| Hetzner AX162-1 dedicated (EPYC 9454P) | 48 (96 threads) | €1.171/h incl. VAT **+ €361.76 setup** | ≈0.03 + setup | [Hetzner Robot](https://robot.hetzner.com/order), [price list](https://docs.hetzner.com/general/infrastructure-and-availability/price-adjustment/) |
| Hetzner AX162-1-LTD (limited stock) | 48 | €0.608/h incl. VAT + €46.41 setup | ≈0.015 + setup | [Hetzner Robot](https://robot.hetzner.com/order) |
| Hetzner Cloud CCX63 | 24 (48 vCPU) | €1.368/h net | ≈0.064 | [Hetzner price list](https://docs.hetzner.com/general/infrastructure-and-availability/price-adjustment/) |
| Vultr Optimized Cloud 96 vCPU, Sydney | 48 | $5.714/h | ≈0.12 | [Vultr pricing](https://www.vultr.com/pricing/) |
| OVH | – | monthly dedicated only (Sydney from A$283/mo + equal install fee; 96-core Scale-a6 from $1,249/mo) | not hourly | [OVH AU](https://www.ovhcloud.com/en-au/bare-metal/prices/), [OVH US](https://us.ovhcloud.com/bare-metal/prices/) |

Hetzner's June 2026 price rise and setup fees make it poor value for a 3-day run; Vultr and AWS
in Sydney cost 5–15× more per core. OVH bills monthly.

**Spot caveat:** Spot machines can be reclaimed at any time. The runner treats a lost worker as an
infrastructure error (exit 3 / ssh failure), the task's gate is retried, and no work is lost because
all commits live on the PC. Keep the PC's own 19 gate slots running throughout.

### GPU: does renting one help?

**No, not for the bottleneck. It is optional and I recommend skipping it.**

- The gates are compilers, PRISM and QEMU. PRISM is a CPU (Java/C) model checker; none of them
  use a GPU. A GPU does not shorten the 10,000 core-hours at all.
- The model work goes to GLM-5.3-Flash through the API, which is not the limit.
- Proof-search models like Goedel-Prover generate **Lean 4** proofs. PBSD's proofs are PRISM
  models, so they would not plug in without a separate Lean track, which is not in the plan.
- A small local triage model (for example classifying gate logs) fits on the PC's RTX 3090 already.

If you still want one for experiments, prices found (Sep 2026 snapshot,
[hostalu](https://hostalu.com/en/gpu-cloud/), [RunPod](https://www.runpod.io/pricing)):

| GPU | RunPod community / secure | Vast.ai median | Lambda |
|---|---|---|---|
| RTX 4090 24 GB | $0.34 / $0.74 | $0.50 | – |
| RTX A6000 48 GB | $0.33 / $0.53–0.59 | $0.47 | $1.09 |
| A100 80 GB | $1.19–1.39 / $1.59 | $0.83 (SXM) | – |
| H100 80 GB | $1.99–2.69 / $2.89–3.49 | $2.77 | $3.29–3.99 |

A 4090 on RunPod community for the whole run would be about 65 h × $0.34 ≈ **US$22 (A$32)**.

### Budget

| Item | Expected | Range | Source |
|---|---|---|---|
| GLM-5.3-Flash API (all providers) | US$420 | $210–940 | `pbsd_run estimate`, see AGENTS.md §8 |
| w1: GCP c3d-highcpu-360 Spot Sydney, 65 h | US$99 | $99 at $1.518/h up to $211 at the US spot rate of $3.25/h | Usage AI, Google |
| w1 boot disk (100 GB pd-balanced, 3 days) and egress | under US$10 | | small; gate traffic is a few GB |
| Calibration run (step 10), ~3 h of w1 + API | US$15 | $10–25 | |
| GPU | US$0 | $22 if you insist | RunPod |
| **Total expected** | **≈ US$545 (A$780)** | | |
| **Budget to set aside (worst API case + double cloud + 15%)** | **US$1,300 (A$1,860)** | | |

Set billing alerts at A$300 on the cloud account and a hard spend limit on every API key (step 3).

## 2. Step by step

You (Odin) do steps 1–4 and the renting yourself; nothing here signs up or rents anything for you.

1. **Create accounts.**
   - Google Cloud (console.cloud.google.com): create a project `pbsd`, add a billing account (AUD card).
   - Z.ai developer platform (z.ai): the main GLM-5.3-Flash provider.
   - Fallback providers, any subset: DeepInfra, Fireworks, OpenRouter. (The runner fails over
     between whichever keys are set.)
   - Optional fallback for CPUs: AWS.
2. **Ask for capacity now; these take time.**
   - **Z.ai:** in the console, request a higher concurrency limit for `glm-5.3-flash` (ask for
     64 concurrent requests). Their review has taken about 10 working days, so do this first.
   - **GCP quotas:** IAM & Admin → Quotas, region `australia-southeast1`: raise
     "C3D CPUs" and "Preemptible CPUs" to at least 400. New projects start far below that, and the
     machine will not start until this is approved. Also request the same in `us-central1` as a backup.
   - **AWS (if used):** Service Quotas → EC2 → "All Standard Spot Instance Requests" to 192 vCPUs in
     `us-west-1`.
3. **Make API keys and set spend limits.** Create one key per provider, set a monthly cap on each
   (Z.ai A$700, each fallback A$300), and keep them in your password manager.
4. **Put the keys in WSL2 on the PC** (not in the repo). In `~/.pbsd/env` (chmod 600):
   ```bash
   export ZAI_API_KEY=...
   export DEEPINFRA_API_KEY=...      # optional
   export FIREWORKS_API_KEY=...      # optional
   export OPENROUTER_API_KEY=...     # optional
   export PBSD_STATE=/run/pbsd/state PBSD_WT=/run/pbsd/wt PBSD_REPO=/run/pbsd/repo PBSD_CHECKPOINT=/mnt/d/pbsd-ckpt
   export PBSD_CONFIG=$HOME/pbsd/docs/plan/tools/runner/config.glm-api-burst.json
   ```
   Add `. ~/.pbsd/env` to `~/.bashrc`. Run `pbsd_run health` to see each provider answer.
5. **Make the runner's ssh key** (WSL2): `ssh-keygen -t ed25519 -f ~/.ssh/pbsd_runner -N ''`.
   Paste `~/.ssh/pbsd_runner.pub` into `tools/cloud/cloud-init.yaml` where it says REPLACE
   (keep that edited copy outside the repo).
6. **Install the gate scripts into the PBSD tree.** Copy `docs/plan/tools/gates/*.sh` to
   `tools/gates/` in the PBSD repository the runner works on (`PBSD_REPO`) and commit. Set the real
   commands once the build layout is fixed: `PBSD_BUILD_CMD`, `PBSD_TEST_CMD`, `PBSD_PROOF_CMD`
   (default: `prism_check.sh` over `proofs/*.pm` + `.props`), `PBSD_QEMU_IMAGE`, `PBSD_QEMU_MARKER`.
   Each gate exits 0 pass, 1 fail, 3 infrastructure error.
7. **Rent w1** (only when steps 2's quota is approved and you are ready to run).
   ```bash
   gcloud compute instances create pbsd-w1 --project pbsd --zone australia-southeast1-b \
     --machine-type c3d-highcpu-360 --provisioning-model SPOT --instance-termination-action DELETE \
     --image-family ubuntu-2404-lts-amd64 --image-project ubuntu-os-cloud --boot-disk-size 100GB \
     --labels pbsd=1 --metadata-from-file user-data=$HOME/.pbsd/cloud-init.yaml
   gcloud compute firewall-rules create pbsd-wg --allow udp:51820 --target-tags pbsd   # only if using WireGuard
   ```
   Bootstrap runs by itself via cloud-init (log: `/var/log/pbsd-bootstrap.log`): it installs the
   toolchain, QEMU and PRISM, creates user `pbsd` with your key, mounts a tmpfs at `/run/pbsd`
   (70% of RAM), creates the bare mirror `/srv/pbsd/pbsd.git` and installs `/opt/pbsd/bin/pbsd-gate`.
   Without cloud-init: `scp -r docs/plan/tools root@w1:/root/pbsd-tools` then
   `sudo PBSD_PUBKEY="$(cat ~/.ssh/pbsd_runner.pub)" bash /root/pbsd-tools/cloud/bootstrap.sh`.
8. **Connect the PC to w1.** Plain ssh with key-only login is enough. WireGuard is optional and gives a
   private 10.66.0.0/24 network, so nothing on your home router needs opening (the PC dials out):
   - In WSL2: `sudo apt install wireguard-tools`, then fill in `tools/cloud/wg-pc.conf.example` →
     `/etc/wireguard/wg0.conf` and `sudo wg-quick up wg0`. Put `wg-worker.conf.example` on w1 the same way.
     (WSL2 kernels since 5.6 include WireGuard. If yours does not, use the Windows WireGuard app with the same file.)
   - Append `tools/cloud/ssh_config.example` to `~/.ssh/config` (aliases `w1` and `pbsd-mirror`, with
     connection multiplexing so ~90 gates share one connection). Test: `ssh w1 nproc`.
9. **Seed the mirror and register the worker.**
   ```bash
   tools/cloud/mirror-setup.sh $PBSD_REPO pbsd-mirror   # pushes the full history once
   tools/cloud/register.sh w1                            # adds "w1 88" to ~/.pbsd/workers.txt
   ```
   The runner re-reads `workers.txt` every 30 s, so workers can be added or removed while it runs.
   For each gate on a worker, the runner commits the task tree, pushes it to the mirror as
   `refs/gates/<task>`, and runs `ssh w1 -- /opt/pbsd/bin/pbsd-gate <role> <task> <sha>`, which checks
   the commit out in tmpfs and runs the same `tools/gates/<role>.sh`.
10. **Calibration run: 50 tasks.** Measure before spending the big money.
    ```bash
    # The first 50 tasks by priority whose dependencies are all inside the set (covers all 6 roles),
    # in a separate state dir so the real run starts clean.
    jq 'reduce (sort_by(.priority)[]) as $t ({ids:{}, out:[]}; . as $s | if ($s.out|length) < 50 and all(($t.deps // [])[]; $s.ids[.] != null) then .ids[$t.id] = true | .out += [$t] else . end) | .out' docs/plan/tasks.json > ~/.pbsd/tasks.calib.json
    PBSD_TASKS=~/.pbsd/tasks.calib.json PBSD_STATE=/run/pbsd/calib pbsd_run init
    PBSD_TASKS=~/.pbsd/tasks.calib.json PBSD_STATE=/run/pbsd/calib pbsd_run run
    PBSD_STATE=/run/pbsd/calib pbsd_run status
    ```
    Check: gates pass on w1 as well as on the PC; real core-minutes per gate; tokens and US$ per task;
    429 rate per provider. Then `pbsd_run estimate burst_cores=180` with the measured numbers. If the
    estimated total is over the budget, stop and adjust before step 11.
11. **Full run.** `pbsd_run run` inside `tmux` so it survives the terminal closing. It resumes from
    the tmpfs database or the HDD checkpoint after any crash or reboot.
12. **Monitor.** `pbsd_run status` (or the live screen) shows overall and per-phase progress bars,
    time left and finish time, cost so far and projected, gate slots (`compile 8/8 qemu 2/2 proof 9/9
    w1 85/88`) and one line per provider. Check twice a day:
    - GCP console → Billing → Reports; Z.ai and fallback usage pages.
    - If w1 is preempted: `gcloud compute instances list --filter=labels.pbsd=1`. Recreate it with the
      step 7 command (same name), rerun steps 9's `mirror-setup.sh` and `register.sh`. The tasks that
      were gating there are retried automatically.
    - If Spot in Sydney keeps getting reclaimed, rent the AWS fallback and register it as `w2`.
13. **Stop.** Ctrl-C the runner (it checkpoints on exit), or let it finish.
14. **Tear down so billing stops.** Same day the run ends:
    ```bash
    tools/cloud/teardown.sh                                         # drains workers, lists what still bills
    gcloud compute instances delete pbsd-w1 --zone australia-southeast1-b --quiet
    gcloud compute disks list --filter=name~pbsd                    # delete anything left
    aws ec2 terminate-instances --instance-ids <id>                 # only if you used AWS
    ```
    Nothing on w1 needs saving: every commit lives on the PC and its HDD checkpoint.
    Then revoke or lower the API key spend caps.

## 3. Cost checklist

- [ ] Billing alert on GCP at A$300 and A$600; spend cap on every API key.
- [ ] Spot price for `c3d-highcpu-360` in `australia-southeast1` checked on the day (expect about US$1.52/h).
- [ ] Calibration run (step 10) done and the estimate re-run with measured numbers.
- [ ] Only one big worker running unless the calibration says the PC + w1 cannot keep the agents busy.
- [ ] No GPU rented (it does not speed up the gates).
- [ ] After the run: `teardown.sh` shows nothing, the instance and its disk are deleted, and the next
      day's billing report shows no compute charges.
