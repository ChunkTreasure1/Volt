project "glm"
	location "."
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"

	targetdir ("bin/" .. outputdir .."/%{prj.name}")
	objdir ("bin-int/" .. outputdir .."/%{prj.name}")

	files
	{
		"glm/**.h",
		"glm/**.cpp",
		"glm/**.hpp",
		"glm/**.inl",
		"**.natvis"
	}

	includedirs
	{
		"."
	}

	defines
	{
		"GLM_FORCE_DEPTH_ZERO_TO_ONE",
		"GLM_FORCE_LEFT_HANDED"
	}

	filter "system:windows"
		systemversion "latest"

		filter "configurations:Debug"
			defines { "VT_DIST", "NDEBUG" }
			runtime "Release"
			optimize "on"

		filter "configurations:Release"
			defines { "VT_DIST", "NDEBUG" }
			runtime "Release"
			optimize "on"

		filter "configurations:Dist"
			defines { "VT_DIST", "NDEBUG" }
			runtime "Release"
			optimize "on"