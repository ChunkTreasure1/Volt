@echo off

pushd ..\
Scripts\data\cloc Source/*/Private Source/*/Public Source/*/PCH --exclude-dir=ThirdParty
popd

PAUSE