@echo off

pushd ..\
call scripts\data\premake5.exe vs2022 %*
popd

REM pushd ..\..\Project
REM call %VOLT_PATH%\vendor\premake5.exe vs2022
REM popd

PAUSE