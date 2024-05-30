project "CoreUtilities"
	location "."
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	targetdir ("../bin/" .. outputdir .."/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .."/%{prj.name}")

	pchheader "cupch.h"
	pchsource "src/cupch.cpp"

	warnings "Extra"

	AddCommonFlags()
	AddCommonWarnings()
	AddCommonLinkOptions()
	AddCommonDefines()

	buildoptions 
	{
		"/bigobj"
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
		
		"%{IncludeDir.glm}",
		"%{IncludeDir.Optick}",
		"%{IncludeDir.tracy}",
		"%{IncludeDir.yaml}",
		"%{IncludeDir.zlib}"
	}

	links
	{
		"YamlCPP",
		"tracy",

		"%{Library.zlib}",
	}

	postbuildcommands
	{
		'{COPY} "../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}/%{prj.name}.dll" "../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Sandbox"'
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

	filter "files:vendor/**.hpp"
		warnings "off"
		pchheader ""
		pchsource ""

	filter "files:vendor/**.inl"
		warnings "off"
		pchheader ""
		pchsource ""

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"VT_PLATFORM_WINDOWS",
			"VTCOREUTIL_BUILD_DLL",

			"NOMINMAX",
			"_HAS_STD_BYTE=0",
			"_WINSOCKAPI_",
			"_WINSOCK_DEPRECATED_NO_WARNINGS",
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
		vectorextensions "AVX2"
		isaextensions { "BMI", "POPCNT", "LZCNT", "F16C" }

	filter "configurations:Dist"
		defines { "VT_DIST", "NDEBUG" }
		buildoptions { "/Ot", "/Ob2" }
		runtime "Release"
		optimize "on"
		symbols "on"
		vectorextensions "AVX2"
		isaextensions { "BMI", "POPCNT", "LZCNT", "F16C" }

		postbuildcommands
		{
			'{COPY} "../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}/%{prj.name}.dll" "../../Engine"'
		}