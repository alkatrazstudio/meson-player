@echo off

set regentry=mesonplayer

reg delete HKCU\Software\Microsoft\Windows\CurrentVersion\Run /v "%regentry%" /f
