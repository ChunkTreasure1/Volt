project "WindowModule"
	location "."
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"
	
	language "C++"
	cppdialect "C++20"

	debugdir "../../Engine"
	targetdir ("../bin/" .. outputdir .."/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .."/%{prj.name}")

	pchheader "windowpch.h"
	pchsource "src/windowpch.cpp"

	warnings "Extra"

	AddCommonFlags()
	AddCommonWarnings()
	AddCommonLinkOptions()
	AddCommonIncludeDirs()
	AddCommonDefines()

	disablewarnings
	{
		"4927"
	}

	linkoptions 
	{
		"/ignore:4098",
		"/ignore:4217",
	}

    defines
    {
        "GLFW_INCLUDE_NONE",
    }

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

		"%{IncludeDir.glm}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.CoreUtilities}",
		"%{IncludeDir.DirectXTex}",
		"%{IncludeDir.RHIModule}",
		"%{IncludeDir.LogModule}",
		"%{IncludeDir.EventSystemModule}",
		"%{IncludeDir.InputModule}",
		"%{IncludeDir.tracy}",
		"%{IncludeDir.GLFW}",
	}

	links
	{
		"GLFW",

		"CoreUtilities",
		"DirectXTex",

		"RHIModule",
		"LogModule",
		"EventSystemModule",
		"InputModule",
		"tracy",
	}

	postbuildcommands
	{
		'{COPY} "../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}/%{prj.name}.dll" "../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Sandbox"'
	}

	postbuildcommands
	{
		'{COPY} "../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}/%{prj.name}.dll" "../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Launcher"'
	}

	filter "system:windows"
		systemversion "latest"
		
		defines
		{
			"VT_PLATFORM_WINDOWS",
			"NOMINMAX",
			"WINDOWMODULE_BUILD_DLL"
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