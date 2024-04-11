project "VoltVulkan"
	location "."
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	targetdir ("../../bin/" .. outputdir .."/%{prj.name}")
	objdir ("../../bin-int/" .. outputdir .."/%{prj.name}")

	pchheader "vkpch.h"
	pchsource "src/vkpch.cpp"

	warnings "Extra"

	AddCommonFlags()
	AddCommonWarnings()
	AddCommonLinkOptions()
	AddCommonIncludeDirs()
	AddCommonDefines()

	files
	{
		"src/**.h",
		"src/**.cpp",
		"src/**.hpp",
	}

	includedirs
	{
		"src/",
		"../VoltRHI/src",

		"%{IncludeDir.GLFW}",
		"%{IncludeDir.ImGui}",

		"%{IncludeDir.vma}",
		"%{IncludeDir.VulkanSDK}",

		"%{IncludeDir.Aftermath}"
	}

	defines
	{
		"OPTICK_ENABLE_GPU_VULKAN",
		"GLFW_DLL",
		"TRACY_IMPORTS"
	}

	links
	{
		"ImGui", -- Should not be here
		"VulkanMemoryAllocator",
		"tracy",
		"GLFW",
		"CoreUtilities",

		"%{Library.dxc}",
		"%{Library.Vulkan}",
		"%{Library.VoltRHI}",
	}

	postbuildcommands
	{
		'{COPY} "../../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}/%{prj.name}.dll" "../../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Sandbox"'
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
		defines 
		{
			"VT_PLATFORM_WINDOWS",
			"VTVK_BUILD_DLL"
		}
		
		filter "configurations:Debug"
			defines 
			{ 
				"VT_DEBUG", 
				"VT_ENABLE_ASSERTS",
				"VT_ENABLE_VALIDATION",
				"VT_ENABLE_PROFILING"
			}

			links
			{
				"%{Library.ShaderC_Debug}",
				"%{Library.ShaderC_Utils_Debug}",
				"%{Library.SPIRV_Cross_Debug}",
				"%{Library.SPIRV_Cross_GLSL_Debug}",
				"%{Library.SPIRV_Tools_Debug}",

				"%{Library.Aftermath}"
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

			links
			{
				"%{Library.ShaderC_Release}",
				"%{Library.ShaderC_Utils_Release}",
				"%{Library.SPIRV_Cross_Release}",
				"%{Library.SPIRV_Cross_GLSL_Release}",

				"%{Library.Aftermath}"
			}

			buildoptions { "/Ot", "/Ob2" }
			runtime "Release"
			optimize "on"
			symbols "on"
			vectorextensions "AVX2"
			isaextensions { "BMI", "POPCNT", "LZCNT", "F16C" }

		filter "configurations:Dist"
			defines { "VT_DIST", "NDEBUG" }
			buildoptions { "/Ot", "/Ob2" }
			runtime "Release"
			optimize "on"
			symbols "on"
			vectorextensions "AVX2"
			isaextensions { "BMI", "POPCNT", "LZCNT", "F16C" }

			links
			{
				"%{Library.ShaderC_Release}",
				"%{Library.ShaderC_Utils_Release}",
				"%{Library.SPIRV_Cross_Release}",
				"%{Library.SPIRV_Cross_GLSL_Release}",
			}

			postbuildcommands
			{
				'{COPY} "../../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}/%{prj.name}.dll" "../../../Engine"'
			}