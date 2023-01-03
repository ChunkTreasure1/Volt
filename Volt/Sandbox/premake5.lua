
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Sandbox"
	location "."
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++latest"
	debugdir "../../Engine"

	targetdir ("../bin/" .. outputdir .."/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .."/%{prj.name}")

	pchheader "sbpch.h"
	pchsource "src/sbpch.cpp"

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
		"/WHOLEARCHIVE:PhysX",
		"/WHOLEARCHIVE:GraphKey"
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
		"Sandbox.rc",
		"Sandbox.aps"
	}

	includedirs
	{
		"src/",
		"../Volt/src/",
		"../Amp/src/",
		"../Game/src/",
		"../GraphKey/src/",

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
		"%{IncludeDir.P4}",

		"%{IncludeDir.ffmpeg}"
	}

    links
    {
        "Volt",
		"GraphKey",
		"Amp",


		"ImGuizmo",
		"ImGuiNodeEditor",

		"Game",

		"d3d11.lib",
		"d3dcompiler.lib",
		"dxguid.lib",
		"crypt32.lib",

		"Ws2_32.lib",
		"Winmm.lib",
		"Version.lib",

		"%{Library.fbxsdk}",
		"%{Library.libxml2}",
		"%{Library.zlib}",
		"%{Library.fmod}",
		"%{Library.fmodstudio}",
		"%{Library.fsbank}",

		"%{Library.P4_client}",
		"%{Library.P4_api}",
		"%{Library.P4_script}",
		"%{Library.P4_script_c}",
		"%{Library.P4_script_curl}",
		"%{Library.P4_script_sqlite}",
		"%{Library.P4_rpc}",
		"%{Library.P4_supp}",

		"%{Library.OpenSSL_Crypto}",
		"%{Library.OpenSSL_SSL}",

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
		["SandboxOnlyDebug"] = "Debug"
	}

	debugargs 
	{
		"../Project/Project.vtproj"
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
			symbols "on"
			optimize "on"

		filter "configurations:Dist"
			defines { "VT_DIST", "NDEBUG" }
			runtime "Release"
			symbols "off"
			optimize "on"
			kind "WindowedApp"

			postbuildcommands
			{
				'{COPY} "../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Sandbox/Sandbox.exe" "../../Engine/"'
			}