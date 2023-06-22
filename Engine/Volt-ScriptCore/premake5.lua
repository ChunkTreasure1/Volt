project "Volt-ScriptCore"
	location "."
	kind "SharedLib"
	language "C#"
	dotnetframework "4.7.2"

	targetdir ("../Scripts")
	objdir ("./Scripts/Intermediates")

	files
	{
		"./Source/**.cs",
		"./Properties/**.cs"
	}

	clr "Unsafe"

	filter "configurations:Debug"
		optimize "Off"
		symbols "Default"
		defines 
		{ 
			"DEBUG"
		}

	filter "configurations:Release"
		optimize "On"
		symbols "Default"
		defines 
		{ 
			"NDEBUG",
			"RELEASE"
		}

	filter "configurations:Dist"
		optimize "Full"
		symbols "Off"
		defines 
		{ 
			"NDEBUG",
			"DIST"
		}