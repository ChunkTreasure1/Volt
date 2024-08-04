project "LogModule"
	location "."
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	targetdir ("../bin/" .. outputdir .."/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .."/%{prj.name}")

	warnings "Extra"

	AddCommonFlags()
	AddCommonWarnings()
	AddCommonLinkOptions()
	AddCommonIncludeDirs()
	AddCommonDefines()

	files
	{
		"src/**.h",
		"src/**.cpp",
		"src/**.hpp",
		"**.natvis",
	}

	includedirs
	{
		"src/",
		"../CoreUtilities/src",

		"%{IncludeDir.spdlog}",
	}

	links
	{
		"tracy",
		"CoreUtilities",
	}

	postbuildcommands
	{
		'{COPY} "../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}/%{prj.name}.dll" "../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Sandbox"'
	}

	postbuildcommands
	{
		'{COPY} "../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}/%{prj.name}.dll" "../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Launcher"'
	}

	filter "files:vendor/**.c"
		flags {"NoPCH"}
		warnings "off"
		pchheader ""
		pchsource ""

	filter "files:vendor/**.cpp"
		flags {"NoPCH"}
		warnings "off"
		pchheader ""
		pchsource ""

	filter "files:vendor/**.h"
		warnings "off"
		pchheader ""
		pchsource ""

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"VT_PLATFORM_WINDOWS",
			"LOGMODULE_BUILD_DLL",
			"NOMINMAX"
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
		runtime "Release"
		optimize "on"
		symbols "on"
		vectorextensions "AVX2"
		isaextensions { "BMI", "POPCNT", "LZCNT", "F16C" }

	filter "configurations:Dist"
		defines 
		{ 
			"VT_DIST", 
			"NDEBUG" 
		}
		runtime "Release"
		optimize "on"
		symbols "off"
		vectorextensions "AVX2"
		isaextensions { "BMI", "POPCNT", "LZCNT", "F16C" }

		postbuildcommands
		{
			'{COPY} "../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}/%{prj.name}.dll" "../../Engine"'
		}
