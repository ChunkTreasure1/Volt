@echo off

pushd ..\
call scripts\data\premake5.exe vs2022
popd

pushd ..\..\Project
call %VOLT_PATH%\vendor\premake5.exe vs2022
popd

PAUSE