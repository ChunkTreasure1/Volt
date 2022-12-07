workspace "Wire"
	architecture "x64"
	startproject "Test"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}
	
	flags
	{
		"MultiProcessorCompile"
	}

	configmap
	{
		["GameOnlyDebug"] = "Release",
		["SandboxOnlyDebug"] = "Release"
	}
	
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "Test"
include "Wire"