workspace "Volt"
	architecture "x64"
	startproject "Sandbox"

	configurations
	{
		"Debug",
		"Release",
		"Dist",
		"GameOnlyDebug"
	}
	
	flags
	{
		"MultiProcessorCompile"
	}
	
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "Dependencies.lua"

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

VoltRootDir = "../../"

group "Core"
include "Volt"
include "Volt-ScriptCore"
include "Amp"

group "Game"
include "Game"

group "Tools"
include "Sandbox"
include "NodeEditor"

group ""
include "Launcher"