project "cityhash"
	kind "StaticLib"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.cc",
		"src/**.h"
	}

	includedirs
	{
		"src/",
		"."
	}

	warnings "off"

	filter "configurations:Debug"
		runtime "Debug"
		optimize "off"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
		symbols "on"

	filter "configurations:Dist"
		runtime "Release"
		optimize "on"
		symbols "off"