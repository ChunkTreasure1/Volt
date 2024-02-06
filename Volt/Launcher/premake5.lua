
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Launcher"
	location "."
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	debugdir "../../Engine"

	targetdir ("../bin/" .. outputdir .."/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .."/%{prj.name}")

	warnings "Extra"

	AddCommonFlags()
	AddCommonWarnings()
	AddCommonLinkOptions()
	AddCommonIncludeDirs()
	AddCommonDefines()

	linkoptions 
	{
		"/ignore:4098",
		"/ignore:4217",
		"/WHOLEARCHIVE:Volt",
		"/WHOLEARCHIVE:PhysX",
		"/WHOLEARCHIVE:GraphKey",
		"/WHOLEARCHIVE:Mosaic"
	}

    defines
    {
        "GLFW_INCLUDE_NONE",
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
		"../VoltRenderer/VoltRHI/src",

        "%{IncludeDir.GLFW}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.yaml}",
		"%{IncludeDir.ImGui}",
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

		"%{IncludeDir.entt}",
		"%{IncludeDir.ffmpeg}",

		"%{IncludeDir.detour}",
		"%{IncludeDir.detourcrowd}",
		"%{IncludeDir.rcdtdebugutils}",
		"%{IncludeDir.detourtilecache}",
		"%{IncludeDir.recast}",
		"%{IncludeDir.fastlz}",

		"%{IncludeDir.VulkanSDK}",
		"%{IncludeDir.vma}",
	}

    links
    {
        "Volt",
		"GraphKey",
		"Amp",
		"meshoptimizer",
		"DiscordSDK",

		"ImGuiNodeEditor",

		"%{Library.fmod}",
		"%{Library.fmodstudio}",
		"%{Library.fsbank}",

		"%{Library.PhysX}",

		"%{Library.AkMemoryMgr}",
		"%{Library.AkSoundEngine}",
		"%{Library.AkStreamMgr}",
		"%{Library.AkMusicEngine}",

		"%{Library.AkRoomVerbFX}",

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
		"%{Library.dxc}",
		"%{Library.METIS}"
    }
	
	debugargs 
	{
		"../Project/Project.vtproj"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"NOMINMAX",
			"_HAS_STD_BYTE=0",
			"VT_PLATFORM_WINDOWS"
		}

		links
		{
			"crypt32.lib",
			"Bcrypt.lib",
		
			"Winmm.lib",
			"Version.lib"
		}

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

			"%{Library.Aftermath}"
		}

	filter "configurations:Release"
		defines { "VT_RELEASE", "NDEBUG" }
		runtime "Release"
		symbols "on"
		optimize "on"
		vectorextensions "AVX2"
		isaextensions { "BMI", "POPCNT", "LZCNT", "F16C" }

		links
		{
			"%{Library.ShaderC_Release}",
			"%{Library.ShaderC_Utils_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}",

			"%{Library.Aftermath}"
		}

	filter "configurations:Dist"
		defines { "VT_DIST", "NDEBUG" }
		runtime "Release"
		symbols "on"
		optimize "on"
		vectorextensions "AVX2"
		isaextensions { "BMI", "POPCNT", "LZCNT", "F16C" }
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