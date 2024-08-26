VoltRootDirectory = os.getenv("VOLT_PATH")
workspace "Project"
	architecture "x64"
	startproject "Project"

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
	
include "Code"