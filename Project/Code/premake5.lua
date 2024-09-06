project "GamePlugin"
	location "."
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	targetdir ("%{prj.location}/../Binaries")
	objdir ("%{prj.location}/../Intermediates")

	files
	{
		"src/**.h",
		"src/**.cpp",
		"**.natvis"
	}

	includedirs
	{
		"src/",
	
		"%{IncludeDir.LogModule}",
		"%{IncludeDir.RHIModule}",
		"%{IncludeDir.JobSystemModule}",
		"%{IncludeDir.EventModule}",
		"%{IncludeDir.AssetSystemModule}",
		"%{IncludeDir.CoreUtilities}",
		"%{IncludeDir.EventSystemModule}",
		"%{IncludeDir.EntitySystemModule}",
		"%{IncludeDir.Volt}",

		"%{IncludeDir.half}",
		"%{IncludeDir.tracy}",
		"%{IncludeDir.unordered_dense}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",
	}

	links
	{
		"LogModule",
		"CoreUtilities",
		"EntitySystemModule",
		"AssetSystemModule",
		"Volt"
	}

	defines
	{
		"TRACY_IMPORTS",  
		"VT_ENABLE_NV_AFTERMATH",
		"VT_PLUGIN_BUILD_DLL"
	}

	disablewarnings
	{
		"4251",
		"4275"
	}

	postbuildcommands
	{
		'{COPYFILE} "../Binaries/%{prj.name}.dll", "../Plugins'
	}

	filter "system:windows"
		systemversion "latest"
		defines 
		{
			"VT_PLATFORM_WINDOWS",
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