project "efsw-static-lib"
	kind "StaticLib"
	language "C++"
	targetdir("./lib")
	includedirs { "include", "src" }
	files { "src/efsw/*.cpp", "src/efsw/platform/win/*.cpp" }
	excludes { "src/efsw/WatcherKqueue.cpp", "src/efsw/WatcherFSEvents.cpp", "src/efsw/WatcherInotify.cpp", "src/efsw/FileWatcherKqueue.cpp", "src/efsw/FileWatcherInotify.cpp", "src/efsw/FileWatcherFSEvents.cpp" }
	filter "configurations:debug"
		defines { "DEBUG" }
		symbols "On"
		targetname "efsw-static-debug"
	filter "configurations:release"
		defines { "NDEBUG" }
		optimize "On"
		targetname "efsw-static-release"
	filter "configurations:relwithdbginfo"
		defines { "NDEBUG" }
		symbols "On"
		optimize "On"
		targetname "efsw-static-reldbginfo"