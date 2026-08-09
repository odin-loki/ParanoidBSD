@echo off
title PBSD Migration (live log)
cd /d "%~dp0\.."
echo Streaming ~/pbsd_run.log from WSL...
echo Close this window to stop watching (driver keeps running).
echo.
wsl -d Ubuntu -- tail -f /home/odin/pbsd_run.log
