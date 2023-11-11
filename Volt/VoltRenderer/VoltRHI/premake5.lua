project "VoltRHI"
	location "."
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"

	targetdir ("../../bin/" .. outputdir .."/%{prj.name}")
	objdir ("../../bin-int/" .. outputdir .."/%{prj.name}")

	pchheader "rhipch.h"
	pchsource "src/rhipch.cpp"

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
		"/ignore:4099",
	}

	files
	{
		"src/**.h",
		"src/**.cpp",
		"src/**.hpp",
	}

	includedirs
	{
		"src/",
		"../VoltD3D12/src",
		"../VoltVulkan/src",
		"../../Volt/src",

		"%{IncludeDir.ImGui}",
		"%{IncludeDir.imgui_notify}",
		"%{IncludeDir.Optick}",

		"%{IncludeDir.Aftermath}"
	}

	defines
	{
		"NOMINMAX",
		"_HAS_STD_BYTE=0",
		"_SILENCE_ALL_CXX20_DEPRECATION_WARNINGS",
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