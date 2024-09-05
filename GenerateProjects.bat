@echo off

pushd Sharpmake
call Sharpmake.Application.exe /sources('../Source/Volt.Main.sharpmake.cs')
popd

PAUSE