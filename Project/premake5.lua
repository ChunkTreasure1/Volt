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

project "GamePlugin"
	location "."
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	targetdir ("%{prj.location}/Binaries")
	objdir ("%{prj.location}/Intermediates")

	files
	{
		"Code/src/**.h",
		"Code/src/**.cpp",
		"Code/**.natvis"
	}

	includedirs
	{
		"Code/src/",
		"%{VoltRootDirectory}/../Volt/Volt/src/Volt/Public",
		"%{VoltRootDirectory}/../Volt/CoreUtilities/src/",
		"%{VoltRootDirectory}/../Volt/LogModule/src/",
		"%{VoltRootDirectory}/../Volt/Volt/vendor/tracy/public/tracy",
	}

	links
	{
		"%{VoltRootDirectory}/../Volt/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/LogModule/LogModule.lib"
	}

	defines
	{
		"TRACY_IMPORTS",
		"VT_ENABLE_NV_AFTERMATH",
		"VT_PLUGIN_BUILD_DLL"
	}

	postbuildcommands
	{
		'{COPYFILE} "Binaries/%{prj.name}.dll", "Plugins'
	}

	filter "system:windows"
		systemversion "latest"
		defines 
		{
			"VT_PLATFORM_WINDOWS",
		}

		filter "configurations:Debug"
			defines 
			{ 
				"VT_DEBUG", 
				"VT_ENABLE_ASSERTS",
				"VT_ENABLE_VALIDATION",
				"VT_ENABLE_PROFILING"
			}

			runtime "Debug"
			optimize "off"
			symbols "on"

		filter "configurations:Release"
			defines 
			{ 
				"VT_RELEASE", 
				"VT_ENABLE_ASSERTS",
				"VT_ENABLE_VALIDATION",
				"VT_ENABLE_PROFILING",
				"NDEBUG"
			}

			buildoptions { "/Ot", "/Ob2" }
			runtime "Release"
			optimize "on"
			symbols "on"

		filter "configurations:Dist"
			defines { "VT_DIST", "NDEBUG" }
			buildoptions { "/Ot", "/Ob2" }
			runtime "Release"
			optimize "on"
			symbols "on"