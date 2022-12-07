project "Test"
	location "."
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"

	targetdir ("bin/" .. outputdir .."/%{prj.name}")
	objdir ("bin-int/" .. outputdir .."/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp",
		"src/**.hpp",
	}

	includedirs
	{
		"src",
		"../Wire/src/",
		"$(VCInstallDir)UnitTest/include"
	}
	
	libdirs
	{
		"$(VCInstallDir)UnitTest/lib"
	}

	filter "system:windows"
		systemversion "latest"

		filter "configurations:Debug"
			defines { "LP_DEBUG", "LP_ENABLE_ASSERTS" }
			runtime "Debug"
			symbols "on"

		filter "configurations:Release"
			defines { "LP_RELEASE" }
			runtime "Release"
			optimize "on"

		filter "configurations:Dist"
			defines { "LP_DIST", "NDEBUG" }
			runtime "Release"
			optimize "on"