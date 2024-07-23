project "Nexus"
	location "."
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"

	targetdir ("../bin/" .. outputdir .."/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .."/%{prj.name}")

	pchheader "nexuspch.h"
	pchsource "src/nexuspch.cpp"

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
		"src/**.hpp"
	}

	includedirs
	{
		"src/",
		"../Volt/src",

		"%{IncludeDir.yaml}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.fmod}",
		"%{IncludeDir.entt}",
	}

	defines
	{
		"PX_PHYSX_STATIC_LIB",
	}

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
			"_WINSOCKAPI_",
			"_WINSOCK_DEPRECATED_NO_WARNINGS",
		}
		
		links 
		{
			"Ws2_32.lib"
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
		defines { "VT_DIST", "NDEBUG" }
		runtime "Release"
		optimize "on"
		symbols "off"
		vectorextensions "AVX2"
		isaextensions { "BMI", "POPCNT", "LZCNT", "F16C" }
