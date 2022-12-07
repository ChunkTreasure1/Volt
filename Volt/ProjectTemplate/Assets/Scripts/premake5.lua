local VoltRootDir = '../../../'

workspace "ProjectTemplate"
	architecture "x64"
	startproject "ProjectTemplate"

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
	
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "ProjectTemplate"
	location "."
	kind "SharedLib"
	language "C#"
	dotnetframework "4.7.2"
	namespace "Volt"

	targetdir ("Binaries")
	objdir ("Intermediates")

	files
	{
		"Source/**.cs",
		"Properties/**.cs"
	}

	links
	{
		"Volt-ScriptCore"
	}

	filter "system:windows"
		systemversion "latest"

		filter "configurations:Debug"
			optimize "Off"
			symbols "Default"

		filter "configurations:Release"
			optimize "On"
			symbols "Default"

		filter "configurations:Dist"
			optimize "Full"
			symbols "Off"

group "Volt"

include (VoltRootDir .. "Volt-ScriptCore/")

group ""