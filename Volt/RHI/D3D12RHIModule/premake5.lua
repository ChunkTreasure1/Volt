project "D3D12RHIModule"
	location "."
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"

	targetdir ("../../bin/" .. outputdir .."/%{prj.name}")
	objdir ("../../bin-int/" .. outputdir .."/%{prj.name}")

	pchheader "dxpch.h"
	pchsource "src/dxpch.cpp"

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
		"%{IncludeDir.RHIModule}",
		"%{IncludeDir.LogModule}",

		"vendor/d3d12",

		"%{IncludeDir.GLFW}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.dxc}"
	}

	defines
	{
		"OPTICK_ENABLE_GPU_D3D12",
		"GLFW_DLL",
		"TRACY_IMPORTS",
		-- "VT_ENABLE_NV_AFTERMATH"
	}

	links
	{
		"ImGui",
		"tracy",
		"GLFW",
		"CoreUtilities",
		"LogModule",

		"d3d12.lib",
		"DXGI.lib",
		"dxguid.lib",

		"%{Library.RHIModule}",
		"%{Library.dxc}",
		"%{Library.Aftermath}"
	}

	dependson
	{
		"RHIModule"
	}

	postbuildcommands
	{
		'{COPY} "../../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}/%{prj.name}.dll" "../../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Sandbox"'
	}

	postbuildcommands
	{
		'{COPY} "../../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}/%{prj.name}.dll" "../../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Launcher"'
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
			"VTDX_BUILD_DLL"
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
			vectorextensions "AVX2"
			isaextensions { "BMI", "POPCNT", "LZCNT", "F16C" }