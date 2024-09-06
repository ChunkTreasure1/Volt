using Sharpmake;
using System;
using System.IO;

namespace VoltSharpmake
{
    [Sharpmake.Generate]
    public class VulkanMemoryAllocator : CommonThirdPartyLibProject
    {
        public VulkanMemoryAllocator() : base()
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "VulkanMemoryAllocator";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            SourceFilesFilters = new Sharpmake.Strings(
                "vma/VulkanMemoryAllocator.h",
                "vma/VulkanMemoryAllocator.cpp"
            );

            string vulkanSDKPath = Path.Combine(Environment.GetEnvironmentVariable("VULKAN_SDK"), "Include");
            conf.IncludePrivatePaths.Add(vulkanSDKPath);
            conf.IncludePaths.Add("[project.RootPath]/[project.Name]");
        }
    }
}
