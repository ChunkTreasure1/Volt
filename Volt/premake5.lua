local VoltRootDirectory = os.getenv("VOLT_PATH")

include "./scripts/premakeCustomization/premakeCommon.lua"
include "./scripts/premakeCustomization/solution_items.lua"
include "Dependencies.lua"

workspace "Volt"
	architecture "x64"
	startproject "Sandbox"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}
	
	flags
	{
		"MultiProcessorCompile"
	}

	buildoptions
	{
		-- "/fsanitize=address"
	}

	solution_items
	{
		".editorconfig"
	}
	
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
include "Volt/vendor/glfw"
include "Volt/vendor/imgui"
include "Volt/vendor/Optick"
include "Volt/vendor/ImGuizmo"
include "Volt/vendor/imgui-node-editor"
include "Volt/vendor/msdf-atlas-gen"
include "Volt/vendor/yaml-cpp"
include "Volt/vendor/DirectXTex"
include "Volt/vendor/efsw"
include "Volt/vendor/meshoptimizer"
include "Volt/vendor/DiscordSDK"
include "Volt/vendor/glm"
include "Volt/vendor/nfd-extended"
include "Volt/vendor/stb_image"
include "Volt/vendor/vma"
include "Volt/vendor/cityhash"
include "Volt/vendor/tracy"
include "Volt/vendor/ufbx"

group "Core"
include "Amp"
include "Volt"
include "Navigation"
include "Nexus"
include "CoreUtilities"
include "Mosaic"
include "LogModule"
include "RenderCoreModule"
include (path.join(VoltRootDirectory, "Volt-ScriptCore"))

group "Core/RHI"
include "RHI/RHIModule"
include "RHI/D3D12RHIModule"
include "RHI/VulkanRHIModule"

group "Tools"
include "Sandbox"

group ""
include "Launcher"