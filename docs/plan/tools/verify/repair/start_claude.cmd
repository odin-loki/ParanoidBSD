@echo off
rem start_claude.cmd <job> [<job> ...]   Claude Message-Batches jobs (cl_watch.sh -> cl_all.sh), jobs live in C:\ParanoidBSD\mined-data\claude\jobs\<name>\ (job.env). Resumable: run it again after a stop.
rem Total cap (CAP in job.env, default USD 150 over ALL Claude spend: tests + every job). Stop cleanly: create C:\ParanoidBSD\mined-data\claude\STOP (or jobs\<name>\STOP)
set R=/mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/docs/plan/tools/verify/repair
start "PBSD claude WATCHER" cmd /k wsl -d Ubuntu -- bash -lc "bash %R%/cl_watch.sh %*"
timeout /t 3 /nobreak > nul
start "PBSD claude LOG" cmd /k wsl -d Ubuntu -- bash -lc "touch /mnt/c/ParanoidBSD/mined-data/claude/watch.log; tail -n 30 -F /mnt/c/ParanoidBSD/mined-data/claude/watch.log"
