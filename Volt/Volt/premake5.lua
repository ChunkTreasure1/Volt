project "Volt"
	location "."
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"

	targetdir ("../bin/" .. outputdir .."/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .."/%{prj.name}")

	pchheader "vtpch.h"
	pchsource "src/vtpch.cpp"

	warnings "Extra"

	AddCommonFlags()
	AddCommonWarnings()
	AddCommonLinkOptions()
	AddCommonIncludeDirs()
	AddCommonDefines()

	buildoptions 
	{
		"/bigobj"
	}

	files
	{
		"src/**.h",
		"src/**.cpp",
		"src/**.hpp",

		"%{IncludeDir.shaderc_glslc}/**.cc",
		"%{IncludeDir.shaderc_glslc}/**.h",

		"%{IncludeDir.shaderc_utils}/**.cc",
		"%{IncludeDir.shaderc_utils}/**.h",

		"%{IncludeDir.tinyddsloader}/**.h",
		"%{IncludeDir.TinyGLTF}/**.h",
	}

	includedirs
	{
		"src/",
		"../Amp/src",
		"../Game/src",
		"../GraphKey/src/",
		"../Navigation/src/",
		"../Nexus/src/",
		
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.yaml}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.TinyGLTF}",
		"%{IncludeDir.tinyddsloader}",
		"%{IncludeDir.imgui_notify}",
		"%{IncludeDir.TGAFbx}",
		"%{IncludeDir.DirectXTK}",
		"%{IncludeDir.fmod}",
		"%{IncludeDir.wwise}",
		"%{IncludeDir.cr}",
		"%{IncludeDir.msdf_atlas_gen}",
		"%{IncludeDir.msdfgen}",
		"%{IncludeDir.PhysX}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.meshoptimizer}",
		"%{IncludeDir.half}",
		"%{IncludeDir.steam}",
		"%{IncludeDir.discord}",
		"%{IncludeDir.NFDExtended}",
		"%{IncludeDir.entt}",

		"%{IncludeDir.glm}",
		"%{IncludeDir.ffmpeg}",
		"%{IncludeDir.mono}",
		"%{IncludeDir.DirectXTex}",
		"%{IncludeDir.efsw}",
		"%{IncludeDir.asio}",

		"%{IncludeDir.recast}",
		"%{IncludeDir.rcdtdebugutils}",

		"%{IncludeDir.detour}",
		"%{IncludeDir.detourcrowd}",
		"%{IncludeDir.detourtilecache}",

		"%{IncludeDir.vma}",
		"%{IncludeDir.VulkanSDK}",
		"%{IncludeDir.shaderc_glslc}",
		"%{IncludeDir.shaderc_utils}"
	}

	links
	{
		"GLFW",
		"ImGui",
		"Optick",
		"Navigation",
		"msdf-atlas-gen",
		"YamlCPP",
		"DirectXTex",
		"efsw-static-lib",
		"Nexus",
		"NFD-Extended",
		"TGAFBX",

		"DirectXTK",
		"stb",
		"VulkanMemoryAllocator"
	}

	defines
	{
		"PX_PHYSX_STATIC_LIB",
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

	filter "files:vendor/**.hpp"
		warnings "off"
		pchheader ""
		pchsource ""

	filter "files:vendor/**.inl"
		warnings "off"
		pchheader ""
		pchsource ""

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"NOMINMAX",
			"_HAS_STD_BYTE=0",
			"_WINSOCKAPI_",
			"_WINSOCK_DEPRECATED_NO_WARNINGS",
		}

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

		buildoptions { "/Ot", "/Ob2" }
		runtime "Release"
		optimize "on"
		symbols "on"

	filter "configurations:Dist"
		defines { "VT_DIST", "NDEBUG" }
		buildoptions { "/Ot", "/Ob2" }
		runtime "Release"
		optimize "on"
		symbols "on"

project "Shaders"
	location "."
	language "C++"
	kind "None"
	
	cppdialect "C++20"

	targetdir ("../bin/" .. outputdir .."/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .."/%{prj.name}")

	files
	{
		"../../Engine/Engine/Shaders/**.hlsl",
		"../../Engine/Engine/Shaders/**.hlslh",
		"../../Engine/Engine/Shaders/**.hlsli",

		"../../Engine/Engine/Shaders/**.glsl",
		"../../Engine/Engine/Shaders/**.glslh",
		"../../Engine/Engine/Shaders/**.glsli",

		"../../Engine/Engine/Shaders/**.h",

		"../../Project/Assets/**.hlsl",
		"../../Project/Assets/**.hlsli"
	}

	filter "configurations:Debug"
		runtime "Debug"
		optimize "off"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
		symbols "on"
		vectorextensions "AVX2"
		isaextensions { "BMI", "POPCNT", "LZCNT", "F16C" }

	filter "configurations:Dist"
		runtime "Release"
		optimize "on"
		symbols "on"
		vectorextensions "AVX2"
		isaextensions { "BMI", "POPCNT", "LZCNT", "F16C" }