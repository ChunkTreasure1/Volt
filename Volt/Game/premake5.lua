
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Game"
	location "."
	kind "StaticLib"
	language "C++"
	cppdialect "C++latest"
	debugdir "../../Build"

	targetdir ("../bin/" .. outputdir .."/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .."/%{prj.name}")

	pchheader "gepch.h"
	pchsource "src/gepch.cpp"

	disablewarnings
	{
		"4005"
	}

	linkoptions 
	{
		"/ignore:4006",
		"/ignore:4099",
		"/ignore:4098"
	}

    defines
    {
		"NOMINMAX"
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
		"../Volt/src/",
		"../Amp/src/",

		"%{IncludeDir.spdlog}",
		"%{IncludeDir.yaml}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.Wire}",
		"%{IncludeDir.Optick}",
		"%{IncludeDir.GEM}",
		"%{IncludeDir.cr}",
		"%{IncludeDir.fmod}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.msdf_atlas_gen}",
		"%{IncludeDir.msdfgen}",
		"%{IncludeDir.PhysX}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.ffmpeg}"
	}

	configmap
	{
		["GameOnlyDebug"] = "Debug",
		["SandboxOnlyDebug"] = "Dist"
	}

	filter "system:windows"
		systemversion "latest"

		filter "configurations:Debug"
			defines { "VT_DEBUG" }
			runtime "Debug"
			symbols "on"
			optimize "off"

		filter "configurations:Release"
			defines { "VT_RELEASE", "NDEBUG" }
			runtime "Release"
			optimize "on"
			symbols "on"

		filter "configurations:Dist"
			defines { "VT_DIST", "NDEBUG" }
			runtime "Release"
			optimize "on"
			symbols "off"