project "GraphKey"
	location "."
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"

	targetdir ("../bin/" .. outputdir .."/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .."/%{prj.name}")

	pchheader "gkpch.h"
	pchsource "src/gkpch.cpp"

	warnings "Extra"

	flags
	{
		"FatalWarnings"
	}

	disablewarnings
	{
		"4005",
		"4201",
		"4100"
	}

	linkoptions 
	{
		"/ignore:4006",
		"/ignore:4099"
	}

	files
	{
		"src/**.h",
		"src/**.cpp",
		"src/**.hpp"
	}

	includedirs
	{
		"src/",
		"../Volt/src/",
		"../Amp/src/",
		"../Nexus/src",

		"%{IncludeDir.yaml}",
		"%{IncludeDir.Optick}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.Wire}",

		"%{IncludeDir.glm}",
		"%{IncludeDir.fmod}",

		"%{IncludeDir.ImGui}",
		"%{IncludeDir.imgui_notify}",
		"%{IncludeDir.GLFW}",
		
		"%{IncludeDir.VulkanSDK}",
		"%{IncludeDir.vma}",
		"%{IncludeDir.half}",
		"%{IncludeDir.entt}",
	}

	defines
	{
		"_SILENCE_ALL_CXX20_DEPRECATION_WARNINGS",
		"PX_PHYSX_STATIC_LIB",

		"GLM_FORCE_DEPTH_ZERO_TO_ONE",
		"GLM_FORCE_LEFT_HANDED"
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
			"NOMINMAX",
			"_HAS_STD_BYTE=0"
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
