<<<<<<<< HEAD:Volt/Amp/premake5.lua
project "Amp"
========
project "GraphKey"
>>>>>>>> main:Volt/GraphKey/premake5.lua
	location "."
	kind "StaticLib"
	language "C++"
	cppdialect "C++latest"

	targetdir ("../bin/" .. outputdir .."/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .."/%{prj.name}")

<<<<<<<< HEAD:Volt/Amp/premake5.lua
	pchheader "amppch.h"
	pchsource "src/amppch.cpp"
========
	pchheader "gkpch.h"
	pchsource "src/gkpch.cpp"
>>>>>>>> main:Volt/GraphKey/premake5.lua

	disablewarnings
	{
		"4005"
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
<<<<<<<< HEAD:Volt/Amp/premake5.lua
		"../Volt/src",

		"%{IncludeDir.yaml}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.Optick}",
		"%{IncludeDir.fmod}",
========
		"../Volt/src/",

		"%{IncludeDir.yaml}",
		"%{IncludeDir.Optick}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.Wire}",

>>>>>>>> main:Volt/GraphKey/premake5.lua
		"%{IncludeDir.GEM}"
	}

	defines
	{
		"NOMINMAX",
		"_HAS_STD_BYTE=0",
		"_SILENCE_ALL_CXX20_DEPRECATION_WARNINGS",
		"PX_PHYSX_STATIC_LIB"
	}

	configmap
	{
		["GameOnlyDebug"] = "Dist",
		["SandboxOnlyDebug"] = "Dist"
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

		filter "configurations:Dist"
			defines { "VT_DIST", "NDEBUG" }
			runtime "Release"
			optimize "on"
			symbols "off"