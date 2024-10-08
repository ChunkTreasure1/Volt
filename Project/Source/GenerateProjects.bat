@echo off
REM Enable delayed variable expansion
setlocal enabledelayedexpansion

REM Change directory to Sharpmake
pushd Sharpmake

REM Call the Sharpmake application with the processed arguments
call Sharpmake.Application.exe /sources('../Game.Main.sharpmake.cs')

REM Return to the original directory
popd

REM Pause to view any messages
PAUSE