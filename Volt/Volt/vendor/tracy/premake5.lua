project "tracy"
	kind "SharedLib"
	language "C++"
	staticruntime "off"
	warnings "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"public/*.cpp",
		"public/*.hpp",
	}

	includedirs
	{
		"public/",
		"public/tracy"
	}

	defines
	{
		"TRACY_ENABLE",
		"TRACY_ON_DEMAND",
		"TRACY_EXPORTS"
	}

	postbuildcommands
	{
		'{COPY} "bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}/"%{prj.name}".dll" "../../../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Sandbox"'
	}

	postbuildcommands
	{
		'{COPY} "bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}/"%{prj.name}".dll" "../../../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/Launcher"'
	}

	warnings "off"

	filter "system:windows"
		systemversion "latest"
		cppdialect "C++20"

	filter "system:linux"
		pic "On"
		systemversion "latest"
		cppdialect "C++20"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		runtime "Release"
		optimize "on"

		postbuildcommands
		{
			'{COPY} "bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}/%{prj.name}.dll" "../../../../Engine"'
		}