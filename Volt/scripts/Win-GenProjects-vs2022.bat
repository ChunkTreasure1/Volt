@echo off

pushd ..\
call scripts\data\premake5.exe vs2022
popd


PAUSE