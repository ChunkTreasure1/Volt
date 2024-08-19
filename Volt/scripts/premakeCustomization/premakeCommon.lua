function AddCommonFlags()
	flags
	{
		"FatalWarnings"
	}
end

function AddCommonWarnings()
    disablewarnings
    {
		"4201",
		"4100",
		"4505",
        "4005",
		"4251",
		"4275"
    }
end

function AddCommonLinkOptions()
    linkoptions 
	{
		"/ignore:4006",
		"/ignore:4099",
	}
end

function AddCommonIncludeDirs()
    includedirs
    {
        "%{IncludeDir.CoreUtilities}",

        "%{IncludeDir.glm}",
		"%{IncludeDir.Optick}",
		"%{IncludeDir.tracy}",
		"%{IncludeDir.unordered_dense}"
    }
end

function AddCommonDefines()
    defines
	{
		"_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS",
		"_SILENCE_ALL_CXX20_DEPRECATION_WARNINGS",
		"_CRT_SECURE_NO_WARNINGS",
		"_HAS_STD_BYTE=0",
		"_USE_MATH_DEFINES",
		"_WINSOCK_DEPRECATED_NO_WARNINGS",

		"NOMINMAX",

		"GLM_FORCE_DEPTH_ZERO_TO_ONE",
		"GLM_FORCE_LEFT_HANDED",

		"TRACY_ENABLE",
		"TRACY_ON_DEMAND"
	}
end