@echo off
rem start_step4_repaired.cmd [budget-usd]   DeepSeek review (step 4) of the REPAIRED annotations, supervised. Data: C:\ParanoidBSD\mined-data\verify-repair\xcheck . Resumable. Stop: create xcheck\STOP there.
set BUDGET=%1
if "%BUDGET%"=="" set BUDGET=70
set W=C:\ParanoidBSD\mined-data\verify-repair
if exist %W%\xcheck\STOP del %W%\xcheck\STOP
set V=/mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/docs/plan/tools/verify
start "PBSD repaired-review WATCHER" cmd /k wsl -d Ubuntu -- bash -lc "export S4_V=/mnt/c/ParanoidBSD/mined-data/verify-repair S4_X=/mnt/c/ParanoidBSD/mined-data/verify-repair/xcheck; bash %V%/step4_watch.sh %BUDGET% 160"
timeout /t 3 /nobreak > nul
start "PBSD repaired-review LOG" cmd /k wsl -d Ubuntu -- bash -lc "mkdir -p /mnt/c/ParanoidBSD/mined-data/verify-repair/xcheck; tail -n 30 -F /mnt/c/ParanoidBSD/mined-data/verify-repair/xcheck/step4.log /mnt/c/ParanoidBSD/mined-data/verify-repair/xcheck/watch.log"
