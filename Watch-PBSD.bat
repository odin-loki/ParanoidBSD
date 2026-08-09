@echo off
title PBSD Migration Monitor
cd /d "%~dp0"
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\watch-migration.ps1"
