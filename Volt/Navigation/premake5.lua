project "Navigation"
	location "."
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"

	targetdir ("../bin/" .. outputdir .."/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .."/%{prj.name}")

	pchheader "nvpch.h"
	pchsource "src/nvpch.cpp"

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
		"src/**.hpp",
		
		"vendor/Recast/**.h",
		"vendor/Recast/**.cpp",

		"vendor/DebugUtils/**.h",
		"vendor/DebugUtils/**.cpp",

		"vendor/Detour/**.h",
		"vendor/Detour/**.cpp",
		"vendor/DetourCrowd/**.h",
		"vendor/DetourCrowd/**.cpp",
		"vendor/DetourTileCache/**.h",
		"vendor/DetourTileCache/**.cpp",
		
		"vendor/Fastlz/**.h",
		"vendor/Fastlz/**.c"
	}

	includedirs
	{
		"src/",
		"../Volt/src",
		"../Nexus/src",

		"%{IncludeDir.yaml}",
		"%{IncludeDir.Optick}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.Wire}",

		"%{IncludeDir.PhysX}",

		"%{IncludeDir.glm}",
		"%{IncludeDir.fmod}",

		"%{IncludeDir.ImGui}",
		"%{IncludeDir.imgui_notify}",
		"%{IncludeDir.GLFW}",

		"%{IncludeDir.recast}",
		"%{IncludeDir.rcdtdebugutils}",

		"%{IncludeDir.detour}",
		"%{IncludeDir.detourcrowd}",
		"%{IncludeDir.detourtilecache}",
		"%{IncludeDir.fastlz}",

		"%{IncludeDir.half}",
		"%{IncludeDir.entt}",

		"%{IncludeDir.vma}",
		"%{IncludeDir.VulkanSDK}",
		"%{IncludeDir.shaderc_glslc}",
		"%{IncludeDir.shaderc_utils}"
	}

	defines
	{
		"GLM_FORCE_DEPTH_ZERO_TO_ONE",
		"GLM_FORCE_LEFT_HANDED",

		"_USE_MATH_DEFINES",
		"_SILENCE_ALL_CXX20_DEPRECATION_WARNINGS",
		"_CRT_SECURE_NO_WARNINGS"
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
			"NOMINMAX",
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
