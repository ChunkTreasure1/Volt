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

		"%{IncludeDir.Volt}",
		"%{IncludeDir.Nexus}",
		"%{IncludeDir.RHIModule}",
		"%{IncludeDir.Mosaic}",
		"%{IncludeDir.LogModule}",
		"%{IncludeDir.RenderCoreModule}",
		"%{IncludeDir.AssetSystemModule}",

		"%{IncludeDir.yaml}",
		"%{IncludeDir.spdlog}",

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
			"VT_PLATFORM_WINDOWS"
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
