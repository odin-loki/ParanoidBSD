@echo off
rem start_pilot.cmd   S4 pilot (200 functions) in its own windows: WATCHER (runs and supervises, holds the WSL session open) and LOG (live view).
rem Data: C:\ParanoidBSD\mined-data\pilot (persistent). Stop cleanly: create C:\ParanoidBSD\mined-data\pilot\STOP. Resume: delete STOP, run this again.
rem Morph spend cap USD 40 (enforced per call in morph_call.sh from morph_ledger.tsv).
if exist C:\ParanoidBSD\mined-data\pilot\STOP del C:\ParanoidBSD\mined-data\pilot\STOP
start "PBSD pilot WATCHER" cmd /k wsl -d Ubuntu -- bash /mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79/docs/plan/lib/code-mined/pilot/pilot_watch.sh 8 4
timeout /t 5 /nobreak > nul
start "PBSD pilot LOG" cmd /k wsl -d Ubuntu -- bash -lc "tail -n 30 -F /mnt/c/ParanoidBSD/mined-data/pilot/watch.log /mnt/c/ParanoidBSD/mined-data/pilot/run.log"
