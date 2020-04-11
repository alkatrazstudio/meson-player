@echo off

set progname=\"%~dp0mesonplayer.exe\"
set regentry=mesonplayer

reg add HKCU\Software\Microsoft\Windows\CurrentVersion\Run /f /v "%regentry%" /t REG_SZ /d "%progname%"
