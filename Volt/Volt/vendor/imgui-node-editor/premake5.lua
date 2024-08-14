project "ImGuiNodeEditor"
	kind "StaticLib"
	language "C++"
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"crude_json.cpp",
		"crude_json.h",
		"imgui_bezier_math.h",
		"imgui_bezier_math.inl",
		"imgui_canvas.cpp",
		"imgui_canvas.h",
		"imgui_extra_math.h",
		"imgui_extra_math.inl",
		"imgui_node_editor.cpp",
		"imgui_node_editor.h",
		"imgui_node_editor_api.cpp",
		"imgui_node_edtior_internal.h",
		"imgui_node_edtior_internal.inl",
		"builders.cpp",
		"builders.h",
		"**.natvis"
	}

	includedirs
	{
		"%{IncludeDir.ImGui}"
	}

	configmap
	{
		["GameOnlyDebug"] = "Dist",
		["SandboxOnlyDebug"] = "Dist"
	}

	filter "system:windows"
		systemversion "latest"
		cppdialect "C++17"

	filter "system:linux"
		pic "On"
		systemversion "latest"
		cppdialect "C++17"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		runtime "Release"
		optimize "on"