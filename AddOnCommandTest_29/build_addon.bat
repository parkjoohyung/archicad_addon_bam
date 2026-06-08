@echo off
call "D:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
"D:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -B Build -G "Visual Studio 17 2022" -A x64
"D:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --build Build --config Release

if %ERRORLEVEL% EQU 0 (
    echo [SUCCESS] Build complete.
    copy /Y "Build\Release\ac_bam_AC29_v1.0.1.apx" "D:\Program Files\GRAPHISOFT\Archicad 29\Add-Ons\ac_bam_AC29_v1.0.1.apx"
) else (
    echo [ERROR] Build failed.
)
