@echo off

pushd ..\
scripts\data\cloc Sandbox/src Volt/src/ Launcher/src/ RenderCoreModule/src Navigation/src Amp/src RHI/RHIModule/src RHI/D3D12RHIModule/src RHI/VulkanRHIModule/src Mosaic/src CoreUtilities/src LogModule/src JobSystemModule/src AssetSystemModule/src EventSystemModule/src InputModule/src WindowModule/src ../Engine/Engine/Shaders
popd

PAUSE