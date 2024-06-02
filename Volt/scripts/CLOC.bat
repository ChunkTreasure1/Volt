@echo off

pushd ..\
scripts\data\cloc Sandbox/src Volt/src/ Launcher/src/ GraphKey/src Navigation/src Amp/src Nexus/src ../Engine/Volt-ScriptCore/Source VoltRenderer/VoltRHI/src VoltRenderer/VoltD3D12/src VoltRenderer/VoltVulkan/src Mosaic/src CoreUtilities/src
popd

PAUSE