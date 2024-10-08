@echo off

pushd ..\
Scripts\data\cloc Engine/Source/*/Private Engine/Source/*/Public Engine/Source/*/PCH --exclude-dir=ThirdParty
popd

PAUSE