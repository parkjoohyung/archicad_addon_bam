@echo on
call "D:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
"D:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -B Build -G "Visual Studio 17 2022" -A x64
"D:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --build Build --config Release

if %ERRORLEVEL% EQU 0 (
    echo [SUCCESS] Build complete.
    copy /Y "Build\Release\ac_bam_AC27_v1.0.1.apx" "C:\Program Files\GRAPHISOFT\Archicad 27\애드온\ac_bam_AC27_v1.0.1.apx"
) else (
    echo [ERROR] Build failed.
)
