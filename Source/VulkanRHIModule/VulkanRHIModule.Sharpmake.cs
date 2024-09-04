using System;
using System.IO;

namespace Volt
{
    [Sharpmake.Generate]
    public class VulkanRHIModule : CommonVoltDllProject
    {
        public VulkanRHIModule() 
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "VulkanRHIModule";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.SolutionFolder = "Engine/RHI";

            conf.PrecompHeader = "vkpch.h";
            conf.PrecompSource = "vkpch.cpp";

            conf.AddPublicDependency<RHIModule>(target);
            conf.AddPublicDependency<GLFW>(target);
            conf.AddPublicDependency<LogModule>(target);
            conf.AddPublicDependency<imgui>(target);

            conf.AddPrivateDependency<VulkanMemoryAllocator>(target);
            conf.AddPrivateDependency<Aftermath>(target);
            conf.AddPrivateDependency<DXC>(target);

            string vulkanSDKPath = Path.Combine(Environment.GetEnvironmentVariable("VULKAN_SDK"), "Include");
            conf.IncludePrivatePaths.Add(vulkanSDKPath);

            string vulkanSDKLibPath = Path.Combine(Environment.GetEnvironmentVariable("VULKAN_SDK"), "Lib");
            conf.LibraryPaths.Add(vulkanSDKLibPath);
            conf.LibraryFiles.Add("vulkan-1.lib");
        }

        public override void ConfigureDebug(Configuration conf, CommonTarget target)
        {
            base.ConfigureDebug(conf, target);

            string vulkanSDKPath = Path.Combine(Environment.GetEnvironmentVariable("VULKAN_SDK"), "Lib");
            conf.LibraryPaths.Add(vulkanSDKPath);

            conf.LibraryFiles.Add("shaderc_sharedd.lib");
            conf.LibraryFiles.Add("shaderc_utild.lib");
            conf.LibraryFiles.Add("spirv-cross-cored.lib");
            conf.LibraryFiles.Add("spirv-cross-glsld.lib");
            conf.LibraryFiles.Add("SPIRV-Toolsd.lib");
        }

        public override void ConfigureRelease(Configuration conf, CommonTarget target)
        {
            base.ConfigureRelease(conf, target);

            string vulkanSDKPath = Path.Combine(Environment.GetEnvironmentVariable("VULKAN_SDK"), "Lib");
            conf.LibraryPaths.Add(vulkanSDKPath);

            conf.LibraryFiles.Add("shaderc_shared.lib");
            conf.LibraryFiles.Add("shaderc_util.lib");
            conf.LibraryFiles.Add("spirv-cross-core.lib");
            conf.LibraryFiles.Add("spirv-cross-glsl.lib");
            conf.LibraryFiles.Add("SPIRV-Tools.lib");
        }

        public override void ConfigureDist(Configuration conf, CommonTarget target)
        {
            base.ConfigureDist(conf, target);

            string vulkanSDKPath = Path.Combine(Environment.GetEnvironmentVariable("VULKAN_SDK"), "Lib");
            conf.LibraryPaths.Add(vulkanSDKPath);

            conf.LibraryFiles.Add("shaderc_shared.lib");
            conf.LibraryFiles.Add("shaderc_util.lib");
            conf.LibraryFiles.Add("spirv-cross-core.lib");
            conf.LibraryFiles.Add("spirv-cross-glsl.lib");
            conf.LibraryFiles.Add("SPIRV-Tools.lib");
        }

        public override void ConfigureClangCl(Configuration conf, CommonTarget target)
        {
            base.ConfigureClangCl(conf, target);

            conf.AdditionalCompilerOptions.Add(
                "-Wno-switch",
                "-Wno-delete-non-abstract-non-virtual-dtor"
            );
        }
    }
}
