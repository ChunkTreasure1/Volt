project "VoltLauncher"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++17"
   targetdir "bin/%{cfg.buildcfg}"
   staticruntime "off"
   debugdir "../App"

   files { "src/**.h", "src/**.cpp", "src/**.c" }

   includedirs
   {
      "../vendor/imgui",
      "../vendor/glfw/include",
      "../vendor/yaml-cpp/include",
      "../vendor/NFD-Extended/src/include",

      "../Walnut/Source",
      "../Walnut/Platform/GUI",

      "%{IncludeDir.VulkanSDK}",
      "%{IncludeDir.glm}",
      "%{IncludeDir.curl}"
   }

   links
   {
       "Walnut",
       "yaml-cpp",
       "NFD-Extended",
       
       "%{Library.curl}",
       "Normaliz.lib",
       "Ws2_32.lib",
       "Crypt32.lib",
       "advapi32.lib",
       "wldap32.lib",
       "Shell32.lib"
   }

   targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
   objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

   filter "system:windows"
      systemversion "latest"
      defines { "WL_PLATFORM_WINDOWS" }

   filter "configurations:Debug"
      defines { "WL_DEBUG" }
      runtime "Debug"
      symbols "On"

   filter "configurations:Release"
      defines { "WL_RELEASE" }
      runtime "Release"
      optimize "On"
      symbols "On"

   filter "configurations:Dist"
      kind "WindowedApp"
      defines { "WL_DIST" }
      runtime "Release"
      optimize "On"
      symbols "Off"