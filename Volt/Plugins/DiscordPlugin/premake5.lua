project "DiscordPlugin"
	location "."
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	targetdir ("../../bin/" .. outputdir .."/%{prj.name}")
	objdir ("../../bin-int/" .. outputdir .."/%{prj.name}")

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
		
		"vendor/DiscordSDK/**.cpp",
		"vendor/DiscordSDK/**.h",
	}

	includedirs
	{
		"src/",
		"vendor/DiscordSDK/include/discord",

		"%{IncludeDir.LogModule}",
		"%{IncludeDir.RHIModule}",
		"%{IncludeDir.JobSystemModule}",
		"%{IncludeDir.EventSystemModule}",
		"%{IncludeDir.Volt}",
		"%{IncludeDir.Volt}/Volt/Public",

		"%{IncludeDir.half}",
	}

	links
	{
		"LogModule",
		"CoreUtilities",
		"EventSystemModule",

		"vendor/DiscordSDK/lib/discord_game_sdk.dll.lib"
	}

	defines
	{
		"TRACY_IMPORTS",
		"VT_ENABLE_NV_AFTERMATH",
		"VT_PLUGIN_BUILD_DLL"
	}

	postbuildcommands
	{
		'{MKDIR} "../../../Engine/Plugins/%{prj.name}"',
		'{COPYFILE} "../../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}/%{prj.name}.dll" "../../../Engine/Plugins/%{prj.name}"',
		'{COPYFILE} "%{prj.name}.vtconfig" "../../../Engine/Plugins/%{prj.name}"',
		'{COPYFILE} "../../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}/%{prj.name}.dll" "../../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Sandbox"',
		'{COPYFILE} "../../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}/%{prj.name}.dll" "../../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Launcher"'
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
			"VTRC_BUILD_DLL"
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