
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Launcher"
	location "."
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++latest"
	debugdir "../../Engine"

	targetdir ("../bin/" .. outputdir .."/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .."/%{prj.name}")

	disablewarnings
	{
		"4005"
	}

	linkoptions 
	{
		"/ignore:4006",
		"/ignore:4099",
		"/ignore:4098",
		"/ignore:4217",
		"/WHOLEARCHIVE:Volt",
		"/WHOLEARCHIVE:Game",
		"/WHOLEARCHIVE:PhysX"
	}

    defines
    {
        "GLFW_INCLUDE_NONE",
		"NOMINMAX",
		"_HAS_STD_BYTE=0"
    }

	files
	{
		"src/**.h",
		"src/**.cpp",
		"src/**.hpp",

		"resource.h",
		"Launcher.rc",
		"Launcher.aps"
	}

	includedirs
	{
		"src/",
		"../Volt/src/",
		"../Game/src/",

        "%{IncludeDir.GLFW}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.yaml}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.Wire}",
		"%{IncludeDir.Optick}",
		"%{IncludeDir.imgui_notify}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.fbxsdk}",
		"%{IncludeDir.fmod}",
		"%{IncludeDir.cr}",
		"%{IncludeDir.msdf_atlas_gen}",
		"%{IncludeDir.msdfgen}",
		"%{IncludeDir.PhysX}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.efsw}",

		"%{IncludeDir.imgui_node_editor}",

		"%{IncludeDir.GEM}",
		"%{IncludeDir.ffmpeg}"
	}

    links
    {
        "Volt",

		"Game",

		"d3d11.lib",
		"d3dcompiler.lib",
		"dxguid.lib",
		"Bcrypt.lib",

		"Ws2_32.lib",
		"Winmm.lib",
		"Version.lib",

		"%{Library.fbxsdk}",
		"%{Library.libxml2}",
		"%{Library.zlib}",
		"%{Library.fmod}",
		"%{Library.fmodstudio}",
		"%{Library.fsbank}",

		"%{Library.PhysX}",

		"%{Library.avcodec}",
		"%{Library.avdevice}",
		"%{Library.avfilter}",
		"%{Library.avformat}",
		"%{Library.avutil}",
		"%{Library.swresample}",
		"%{Library.swscale}",

		"%{Library.mono}"
    }

	configmap
	{
		["GameOnlyDebug"] = "Dist",
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
			kind "WindowedApp"

			postbuildcommands
			{
				'{COPY} "../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Launcher/Launcher.exe" "../../Engine/"'
			}