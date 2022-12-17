project "Volt"
	location "."
	kind "StaticLib"
	language "C++"
	cppdialect "C++latest"

	targetdir ("../bin/" .. outputdir .."/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .."/%{prj.name}")

	pchheader "vtpch.h"
	pchsource "src/vtpch.cpp"

	disablewarnings
	{
		"4005"
	}

	linkoptions 
	{
		"/ignore:4006",
		"/ignore:4099"
	}

	files
	{
		"src/**.h",
		"src/**.cpp",
		"src/**.hpp",

		"vendor/DirectXTK/**.h",
		"vendor/DirectXTK/**.cpp",

		"vendor/stb_image/**.cpp",
		"vendor/stb_image/**.h",

		"vendor/cr/**.h"
	}

	includedirs
	{
		"src/",
		"../Game/src",
		"../GraphKey/src/",

		"%{IncludeDir.GLFW}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.yaml}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.Wire}",
		"%{IncludeDir.Optick}",
		"%{IncludeDir.TinyGLTF}",
		"%{IncludeDir.imgui_notify}",
		"%{IncludeDir.fbxsdk}",
		"%{IncludeDir.DirectXTK}",
		"%{IncludeDir.fmod}",
		"%{IncludeDir.cr}",
		"%{IncludeDir.msdf_atlas_gen}",
		"%{IncludeDir.msdfgen}",
		"%{IncludeDir.PhysX}",
		"%{IncludeDir.stb_image}",

		"%{IncludeDir.GEM}",
		"%{IncludeDir.ffmpeg}",
		"%{IncludeDir.mono}",
		"%{IncludeDir.DirectXTex}"
	}

	links
	{
		"GLFW",
		"ImGui",
		"Wire",
		"Optick",
		"msdf-atlas-gen",
		"YamlCPP",
		"DirectXTex",
		"GraphKey"
	}

	defines
	{
		"NOMINMAX",
		"_HAS_STD_BYTE=0",
		"_SILENCE_ALL_CXX20_DEPRECATION_WARNINGS",
		"PX_PHYSX_STATIC_LIB"
	}

	configmap
	{
		["GameOnlyDebug"] = "Dist",
		["SandboxOnlyDebug"] = "Dist"
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
			runtime "Release"
			optimize "on"
			symbols "on"

		filter "configurations:Dist"
			defines { "VT_DIST", "NDEBUG" }
			runtime "Release"
			optimize "on"
			symbols "off"

project "Shaders"
	location "."
	language "C++"
	kind "None"
	
	cppdialect "C++latest"

	targetdir ("../bin/" .. outputdir .."/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .."/%{prj.name}")

	files
	{
		"../../Build/Engine/Shaders/**.hlsl",
		"../../Build/Engine/Shaders/**.hlslh",
		"../../Build/Engine/Shaders/**.hlsli"
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