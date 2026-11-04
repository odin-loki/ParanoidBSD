@echo off
title Stop PBSD Migration
cd /d "%~dp0"
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\run-pbsd-console.ps1" stop
pause
