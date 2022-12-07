project "DirectXTex"
	location "."
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"

	targetdir ("bin/" .. outputdir .."/%{prj.name}")
	objdir ("bin-int/" .. outputdir .."/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp",
		"src/**.hpp",
		"src/**.inl"
	}

	includedirs
	{
		"src/DirectXTex/"
	}

	configmap
	{
		["GameOnlyDebug"] = "Dist",
		["SandboxOnlyDebug"] = "Dist"
	}

	filter "system:windows"
		systemversion "latest"

		filter "configurations:Debug"
			defines { "VT_DEBUG", "VT_ENABLE_ASSERTS" }
			runtime "Debug"
			symbols "on"

		filter "configurations:Release"
			defines { "VT_RELEASE" }
			runtime "Release"
			optimize "on"

		filter "configurations:Dist"
			defines { "VT_DIST", "NDEBUG" }
			runtime "Release"
			optimize "on"