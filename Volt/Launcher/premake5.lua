
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
		"/WHOLEARCHIVE:PhysX",
		"/WHOLEARCHIVE:GraphKey"
	}

    defines
    {
        "GLFW_INCLUDE_NONE",
		"NOMINMAX",
		"_HAS_STD_BYTE=0",

		"GLM_FORCE_DEPTH_ZERO_TO_ONE",
		"GLM_FORCE_SSE2",
		"GLM_FORCE_LEFT_HANDED"
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
		"../Amp/src/",
		"../Game/src/",
		"../GraphKey/src/",
		"../Nexus/src",
		"../Navigation/src/",

        "%{IncludeDir.GLFW}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.yaml}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.Wire}",
		"%{IncludeDir.Optick}",
		"%{IncludeDir.imgui_notify}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.fmod}",
		"%{IncludeDir.cr}",
		"%{IncludeDir.msdf_atlas_gen}",
		"%{IncludeDir.msdfgen}",
		"%{IncludeDir.PhysX}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.efsw}",
		"%{IncludeDir.tinyddsloader}",
		"%{IncludeDir.meshoptimizer}",
		"%{IncludeDir.half}",
		"%{IncludeDir.steam}",
		"%{IncludeDir.discord}",

		"%{IncludeDir.glm}",
		"%{IncludeDir.ffmpeg}",

		"%{IncludeDir.detour}",
		"%{IncludeDir.detourcrowd}",
		"%{IncludeDir.rcdtdebugutils}",
		"%{IncludeDir.detourtilecache}",
		"%{IncludeDir.recast}",
		"%{IncludeDir.fastlz}",

		"%{IncludeDir.VulkanSDK}",
		"%{IncludeDir.vma}"
	}

    links
    {
        "Volt",
		"GraphKey",
		"Amp",
		"meshoptimizer",
		"DiscordSDK",

		"ImGuiNodeEditor",

		"Bcrypt.lib",

		"Ws2_32.lib",
		"Winmm.lib",
		"Version.lib",

		"%{Library.fmod}",
		"%{Library.fmodstudio}",
		"%{Library.fsbank}",

		"%{Library.PhysX}",

		"%{Library.AkMemoryMgr}",
		"%{Library.AkSoundEngine}",
		"%{Library.AkStreamMgr}",
		"%{Library.AkMusicEngine}",

		"%{Library.avcodec}",
		"%{Library.avdevice}",
		"%{Library.avfilter}",
		"%{Library.avformat}",
		"%{Library.avutil}",
		"%{Library.swresample}",
		"%{Library.swscale}",

		"%{Library.mono}",
		"%{Library.steam}",
		"%{Library.discord}",
		
		"%{Library.Vulkan}",
		"%{Library.dxc}"
    }

	configmap
	{
		["GameOnlyDebug"] = "Dist",
		["SandboxOnlyDebug"] = "Dist"
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

			links
			{
				"%{Library.ShaderC_Debug}",
				"%{Library.ShaderC_Utils_Debug}",
				"%{Library.SPIRV_Cross_Debug}",
				"%{Library.SPIRV_Cross_GLSL_Debug}",
				"%{Library.SPIRV_Tools_Debug}",

				"%{Library.VulkanUtils}"
			}

		filter "configurations:Release"
			defines { "VT_RELEASE", "NDEBUG" }
			runtime "Release"
			optimize "on"
			symbols "on"

			links
			{
				"%{Library.ShaderC_Release}",
				"%{Library.ShaderC_Utils_Release}",
				"%{Library.SPIRV_Cross_Release}",
				"%{Library.SPIRV_Cross_GLSL_Release}",
			}

		filter "configurations:Dist"
			defines { "VT_DIST", "NDEBUG" }
			runtime "Release"
			optimize "on"
			symbols "on"
			kind "WindowedApp"

            links
			{
				"%{Library.ShaderC_Release}",
				"%{Library.ShaderC_Utils_Release}",
				"%{Library.SPIRV_Cross_Release}",
				"%{Library.SPIRV_Cross_GLSL_Release}",
			}

			postbuildcommands
			{
				'{COPY} "../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Launcher/Launcher.exe" "../../Engine/"'
			}