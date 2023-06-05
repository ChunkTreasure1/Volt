project "VoltMock"
	location "."
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"

	targetdir ("../../bin/" .. outputdir .."/%{prj.name}")
	objdir ("../../bin-int/" .. outputdir .."/%{prj.name}")

	pchheader "mkpch.h"
	pchsource "src/mkpch.cpp"

	disablewarnings
	{
		"4005"
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
		"../VoltRHI/src"
	}

	defines
	{
		"NOMINMAX",
		"_HAS_STD_BYTE=0",
		"_SILENCE_ALL_CXX20_DEPRECATION_WARNINGS",
	}

	postbuildcommands
	{
		'{COPY} "../../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/VoltMock/VoltMock.dll" "../../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Sandbox/"',
		'{COPY} "../../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/VoltMock/VoltMock.dll" "../../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Launcher/"'
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
				'{COPY} "../../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/VoltMock/VoltMock.dll" "../../../Engine"',
			}