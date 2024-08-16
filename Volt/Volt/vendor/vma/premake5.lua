project "VulkanMemoryAllocator"
	kind "StaticLib"
	language "C++"
	staticruntime "off"
	cppdialect "C++17"
	warnings "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"**.cpp",
		"**.h"
	}

	includedirs
	{
		".",
		"%{IncludeDir.VulkanSDK}",
	}

	filter "system:windows"
		systemversion "latest"

	filter "system:linux"
		pic "On"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		runtime "Release"
		optimize "on"