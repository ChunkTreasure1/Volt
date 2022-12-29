local VoltRootDir = '../../../'

workspace "Project"
	architecture "x64"
	startproject "Project"

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

project "Project"
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

-- group "Volt"

-- include (VoltRootDir .. "Volt-ScriptCore/")

-- group ""