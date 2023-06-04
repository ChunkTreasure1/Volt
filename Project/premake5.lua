VoltRootDirectory = os.getenv("VOLT_PATH")
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

group "Volt"

include (path.join(VoltRootDirectory, "Volt-ScriptCore"))

group ""

include (path.join(VoltRootDirectory, "Engine", "Lua", "Volt.lua"))

project "Project"
	location "."
	kind "SharedLib"
	language "C#"
	dotnetframework "4.7.2"
	namespace "Volt"

	targetdir ("%{prj.location}/Binaries")
	objdir ("%{prj.location}/Intermediates")

	files
	{
		"Assets/**.cs"
	}

	linkAppReferences()

	filter "system:windows"
		systemversion "latest"

		filter "configurations:Debug"
			optimize "Off"
			symbols "Default"
			defines 
			{ 
				"DEBUG"
			}

		filter "configurations:Release"
			optimize "On"
			symbols "Default"
			defines 
			{ 
				"NDEBUG",
				"RELEASE"
			}

		filter "configurations:Dist"
			optimize "Full"
			symbols "Off"
			defines 
			{ 
				"NDEBUG",
				"DIST"
			}