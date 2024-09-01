using System;
using System.IO;

using Sharpmake;

namespace Volt
{
    [Sharpmake.Generate]
    public class GLFW : CommonThirdPartyDllProject
    {
        public GLFW() : base()
        {
            Name = "GLFW";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            // Add specific files
            SourceFilesFilters = new Sharpmake.Strings(
                "include/GLFW/glfw3.h",
                "include/GLFW/glfw3native.h",
                "src/internal.h",
                "src/platform.h",
                "src/mappings.h",
                "src/context.c",
                "src/init.c",
                "src/input.c",
                "src/monitor.c",
                "src/platform.c",
                "src/vulkan.c",
                "src/window.c",
                "src/null_platform.h",
                "src/null_joystick.h",
                "src/null_init.c",
                "src/null_monitor.c",
                "src/null_window.c",
                "src/null_joystick.c");

            conf.IncludePaths.Add("include/");

            conf.Defines.Add("_GLFW_BUILD_DLL");
        }

        public override void ConfigureWin64(Configuration conf, CommonTarget target)
        {
            base.ConfigureWin64(conf, target);

            SourceFilesFilters.Add(
                "src/win32_init.c",
                "src/win32_module.c",
                "src/win32_joystick.c",
                "src/win32_monitor.c",
                "src/win32_time.h",
                "src/win32_time.c",
                "src/win32_thread.h",
                "src/win32_thread.c",
                "src/win32_window.c",
                "src/wgl_context.c",
                "src/egl_context.c",
                "src/osmesa_context.c"
                );

            conf.Defines.Add(
                "_GLFW_WIN32",
                "_CRT_SECURE_NO_WARNINGS"
                );

            conf.LibraryFiles.Add(
                "Dwmapi.lib"
                );
        }

        public override void ConfigureLinux(Configuration conf, CommonTarget target)
        {
            base.ConfigureLinux(conf, target);

            conf.Options.Add(Linux.Options.Compiler.PositionIndependentCode.Enable);

            SourceFilesFilters.Add(
                "src/x11_init.c",
                "src/x11_monitor.c",
                "src/x11_window.c",
                "src/xkb_unicode.c",
                "src/posix_time.c",
                "src/posix_thread.c",
                "src/glx_context.c",
                "src/egl_context.c",
                "src/osmesa_context.c",
                "src/linux_joystick.c"
              );

            conf.Defines.Add(
                "_GLFW_X11"
              );
        }
    }
}
