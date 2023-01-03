local VoltRootDirectory = os.getenv("VOLT_PATH")

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
include "Volt/vendor/efsw"

group "Core"
include "Amp"

include (path.join(VoltRootDirectory, "Volt-ScriptCore"))
include "GraphKey"

group "Game"
include "Game"

group "Tools"
include "Sandbox"

group ""
include "Launcher"