-- premake5.lua
workspace "VoltLauncher"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "VoltLauncher"

   -- Workspace-wide build options for MSVC
   filter "system:windows"
      buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "Build-Walnut-External.lua"
include "VoltLauncher/Build-VoltLauncher.lua"