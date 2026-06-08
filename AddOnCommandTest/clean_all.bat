@echo off
setlocal enabledelayedexpansion
for /r "Src" %%f in (*.cpp *.h *.c *.hpp) do (
    echo Processing %%f
    powershell -Command "$c = Get-Content -Path '%%f' -Raw; [System.IO.File]::WriteAllText('%%f', $c, [System.Text.Encoding]::ASCII)"
)
