@echo off
call "D:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
msbuild Build\AddOn.vcxproj /p:Configuration=Release /v:minimal
