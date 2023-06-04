@echo off

pushd ..\
scripts\data\cloc Sandbox/src Volt/src/ Launcher/src/ Volt/vendor/Wire/Wire/src/ GraphKey/src Amp/src ../Engine/Volt-ScriptCore/Source
popd

PAUSE