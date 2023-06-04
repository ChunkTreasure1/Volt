project "TGAFbx"
	kind "StaticLib"
	language "C++"
	staticruntime "off"
	warnings "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/*.cpp",
		"src/*.h"
	}

	includedirs
	{
		"src",
		"vendor/fbxsdk/include"
	}

	libdirs
	{
		"vendor/fbxsdk/lib/%{cfg.buildcfg}"
	}
	
	links
	{
		"libfbxsdk-md.lib",
		"libxml2-md.lib",
		"zlib-md.lib"
	}

	filter "system:windows"
		systemversion "latest"
		cppdialect "C++20"

	filter "system:linux"
		pic "On"
		systemversion "latest"
		cppdialect "C++20"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		runtime "Release"
		optimize "on"