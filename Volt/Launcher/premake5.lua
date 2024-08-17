
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
		"/WHOLEARCHIVE:Mosaic"
	}

    defines
    {
    }

	files
	{
		"src/**.h",
		"src/**.cpp",
		"src/**.hpp",
		"**.natvis",

		"resource.h",
		"Launcher.rc",
		"Launcher.aps"
	}

	includedirs
	{
		"src/",

		"%{IncludeDir.Volt}",
		"%{IncludeDir.Amp}",
		"%{IncludeDir.Navigation}",
		"%{IncludeDir.Nexus}",
		"%{IncludeDir.Mosaic}",
		"%{IncludeDir.LogModule}",
		"%{IncludeDir.RHIModule}",
		"%{IncludeDir.RenderCoreModule}",
		"%{IncludeDir.JobSystemModule}",
		"%{IncludeDir.AssetSystemModule}",
		"%{IncludeDir.EntitySystemModule}",

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

		"%{IncludeDir.detour}",
		"%{IncludeDir.detourcrowd}",
		"%{IncludeDir.rcdtdebugutils}",
		"%{IncludeDir.detourtilecache}",
		"%{IncludeDir.recast}",
		"%{IncludeDir.fastlz}",

		"%{IncludeDir.VulkanSDK}",
		"%{IncludeDir.vma}",
		"%{IncludeDir.zlib}",

		"%{IncludeDir.EventModule}",
		"%{IncludeDir.WindowModule}",
		"%{IncludeDir.InputModule}",

	}

    links
    {
        "Volt",
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

		"%{Library.mono}",
		"%{Library.steam}",
		"%{Library.discord}",
		
		"%{Library.METIS}",

		"EventModule",
		"WindowModule",
		"InputModule",
    }
	
	debugargs 
	{
		"../Project/Project.vtproj"
	}

	filter "system:windows"
		systemversion "latest"

		postbuildcommands
		{
			'{COPY} "../../Engine/D3D12/D3D12Core.dll" "../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Launcher/D3D12/"',
			'{COPY} "../../Engine/D3D12/D3D12Core.pdb" "../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Launcher/D3D12/"',
			'{COPY} "../../Engine/D3D12/d3d12SDKLayers.dll" "../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Launcher/D3D12/"',
			'{COPY} "../../Engine/D3D12/d3d12SDKLayers.pdb" "../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Launcher/D3D12/"'
		}
		
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

	filter "configurations:Release"
		defines { "VT_RELEASE", "NDEBUG" }
		runtime "Release"
		symbols "on"
		optimize "on"
		vectorextensions "AVX2"
		isaextensions { "BMI", "POPCNT", "LZCNT", "F16C" }

	filter "configurations:Dist"
		defines { "VT_DIST", "NDEBUG" }
		runtime "Release"
		symbols "on"
		optimize "on"
		vectorextensions "AVX2"
		isaextensions { "BMI", "POPCNT", "LZCNT", "F16C" }
		kind "WindowedApp"

		postbuildcommands
		{
			'{COPY} "../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Launcher/Launcher.exe" "../../Engine/"'
		}