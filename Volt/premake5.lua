
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

	solution_items
	{
		".editorconfig"
	}
	
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
include "Volt/vendor/glfw"
include "Volt/vendor/imgui"
include "Volt/vendor/Wire/Wire"
include "Volt/vendor/Optick"
include "Volt/vendor/ImGuizmo"
include "Volt/vendor/imgui-node-editor"
include "Volt/vendor/msdf-atlas-gen"
include "Volt/vendor/yaml-cpp"
include "Volt/vendor/DirectXTex"
include "Volt/vendor/efsw"
include "Volt/vendor/meshoptimizer"
include "Volt/vendor/DiscordSDK"
include "Volt/vendor/gem"
include "Volt/vendor/nfd-extended"
include "Volt/vendor/TGAFbx"

group "Core"
include "Amp"
include "Volt"
include "GraphKey"
include "Navigation"
include "Nexus"
include "../Engine/Volt-ScriptCore"

group "Tools"
include "Sandbox"

group ""
include "Launcher"