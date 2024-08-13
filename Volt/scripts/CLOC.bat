@echo off

pushd ..\
scripts\data\cloc Sandbox/src Volt/src/ Launcher/src/ RenderCoreModule/src Navigation/src Amp/src Nexus/src ../Engine/Volt-ScriptCore/Source RHI/RHIModule/src RHI/D3D12RHIModule/src RHI/VulkanRHIModule/src Mosaic/src CoreUtilities/src LogModule/src JobSystemModule/src ../Engine/Engine/Shaders
popd

PAUSE