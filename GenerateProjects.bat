@echo off
REM Enable delayed variable expansion
setlocal enabledelayedexpansion

REM Initialize the args variable by capturing all arguments
set "args=%*"

REM Change directory to Sharpmake
pushd Sharpmake

REM Call the Sharpmake application with the processed arguments
call Sharpmake.Application.exe /sources('../Source/Volt.Main.sharpmake.cs') %args%

REM Return to the original directory
popd

REM Pause to view any messages
PAUSE