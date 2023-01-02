VoltRootDirectory = os.getenv("VOLT_PATH")
include (path.join(VoltRootDirectory, "Engine", "Lua", "Volt.lua"))

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

group "Volt"

project "Volt-ScriptCore"
	location "%{VoltRootDirectory}/Volt-ScriptCore"
	kind "SharedLib"
	language "C#"
	dotnetframework "4.7.2"

	targetdir ("%{VoltRootDirectory}/Scripts")
	objdir ("%{VoltRootDirectory}/Scripts/Intermediates")

	files
	{
		"%{VoltRootDirectory}/Volt-ScriptCore/Source/**.cs",
		"%{VoltRootDirectory}/Volt-ScriptCore/Properties/**.cs"
	}

	filter "configurations:Debug"
		optimize "Off"
		symbols "Default"

	filter "configurations:Release"
		optimize "On"
		symbols "Default"

	filter "configurations:Dist"
		optimize "Full"
		symbols "Off"

group ""

project "ProjectTemplate"
	location "."
	kind "SharedLib"
	language "C#"
	dotnetframework "4.7.2"
	namespace "Volt"

	targetdir ("%{prj.location}/Binaries")
	objdir ("%{prj.location}/Intermediates")

	files
	{
		"Assets/Scripts/Source/**.cs"
	}

	linkAppReferences()

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