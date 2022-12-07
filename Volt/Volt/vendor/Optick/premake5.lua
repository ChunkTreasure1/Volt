project "Optick"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
    staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp"
	}

	VULKAN_SDK = os.getenv("VULKAN_SDK")
	includedirs
	{
		"%{VULKAN_SDK}/Include"
	}

	defines
	{
		"_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS"
	}

	configmap
	{
		["GameOnlyDebug"] = "Dist",
		["SandboxOnlyDebug"] = "Dist"
	}

	filter "system:windows"
		systemversion "latest"

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
