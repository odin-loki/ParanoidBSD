@echo off
rem start_mining.cmd   Start the whole-corpus mining in its own windows (double-click it, or run it from any prompt).
rem   window 1 "PBSD mining WATCHER"  starts and supervises the miner (restarts it if it dies or stalls, stops it on a credit error); holds a WSL session open
rem   window 2 "PBSD mining LOG"      live view of the miner and watcher logs
rem   window 3 "PBSD mining SYNC"     commits gzip shards of the results to git every 5 minutes
rem All data is in C:\ParanoidBSD\mined-data (not temp). To stop cleanly: create the file C:\ParanoidBSD\mined-data\STOP . To resume: delete it and run this again.
set REPO=/mnt/c/ParanoidBSD/dreamy-mahavira-7e0e79
set DATA=/mnt/c/ParanoidBSD/mined-data
if not exist C:\ParanoidBSD\mined-data mkdir C:\ParanoidBSD\mined-data
if exist C:\ParanoidBSD\mined-data\STOP del C:\ParanoidBSD\mined-data\STOP
start "PBSD mining WATCHER" cmd /k wsl -d Ubuntu -- bash -lc "MINE_TIERS='B C' bash %REPO%/docs/plan/tools/mine/mine_watch.sh"
timeout /t 5 /nobreak > nul
start "PBSD mining LOG" cmd /k wsl -d Ubuntu -- bash -lc "tail -n 40 -F %DATA%/miner.log %DATA%/watch.log"
rem the sync runs in Git Bash on the Windows side (it has the GitHub credentials; WSL has none)
start "PBSD mining SYNC" cmd /k "C:\Program Files\Git\usr\bin\bash.exe" -lc "bash /c/ParanoidBSD/dreamy-mahavira-7e0e79/docs/plan/tools/mine/mine_sync.sh 300"
