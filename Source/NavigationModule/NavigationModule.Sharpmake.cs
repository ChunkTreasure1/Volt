using System;
using System.IO;

namespace VoltSharpmake
{
    [Sharpmake.Generate]
    public class NavigationModule : CommonVoltLibProject
    {
        public NavigationModule()
        {
            Name = "NavigationModule";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.SolutionFolder = "Engine";

            conf.PrecompHeader = "nvpch.h";
            conf.PrecompSource = "nvpch.cpp";

            conf.AddPublicDependency<RecastDetour>(target);

            conf.AddPrivateDependency<LogModule>(target);
            conf.AddPrivateDependency<AssetSystemModule>(target);
            conf.AddPrivateDependency<EntitySystemModule>(target);
            conf.AddPrivateDependency<RenderCoreModule>(target);
            conf.AddPrivateDependency<EventSystemModule>(target);
            conf.AddPrivateDependency<MosaicModule>(target);

            conf.AddPrivateDependency<PhysX>(target);

            conf.IncludePrivatePaths.Add(Path.Combine(Globals.RootDirectory, "Volt", "Public"));
        }
    }
}
