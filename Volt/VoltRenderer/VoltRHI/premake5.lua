project "VoltRHI"
	location "."
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	targetdir ("../../bin/" .. outputdir .."/%{prj.name}")
	objdir ("../../bin-int/" .. outputdir .."/%{prj.name}")

	pchheader "rhipch.h"
	pchsource "src/rhipch.cpp"

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
	}

	includedirs
	{
		"src/",
		"../VoltD3D12/src",

		"%{IncludeDir.ImGui}",
		"%{IncludeDir.imgui_notify}",

		"%{IncludeDir.Aftermath}",
	}

	links
	{
		"ImGui", -- Should not be here
		"tracy",
		"%{Library.CoreUtilities}",
		"%{Library.Aftermath}"
	}

	defines
	{
		"TRACY_IMPORTS",
		"VT_ENABLE_NV_AFTERMATH"
	}

	postbuildcommands
	{
		'{COPY} "../../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}/%{prj.name}.dll" "../../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Sandbox"'
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
			"VTRHI_BUILD_DLL"
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

			postbuildcommands
			{
				'{COPY} "../../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}/%{prj.name}.dll" "../../../Engine"'
			}