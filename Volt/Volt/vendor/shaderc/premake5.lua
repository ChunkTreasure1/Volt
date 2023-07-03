project "shaderc"
	kind "StaticLib"
	language "C++"
	staticruntime "off"
	cppdialect "C++17"
	warnings "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"**.cc",
		"**.h"
	}

	includedirs
	{
		"%{IncludeDir.vma}",
		"%{IncludeDir.VulkanSDK}",
		"%{IncludeDir.shaderc_glslc}",
		"%{IncludeDir.shaderc_utils}"
	}

	defines
	{
		"ENABLE_HLSL"
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