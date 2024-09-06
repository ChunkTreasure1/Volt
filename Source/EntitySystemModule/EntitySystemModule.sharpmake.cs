using System;
using System.IO;

namespace VoltSharpmake
{
    [Sharpmake.Generate]
    public class EntitySystemModule : CommonVoltDllProject
    {
        public EntitySystemModule()
        {
            Name = "EntitySystemModule";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.SolutionFolder = "Engine";

            conf.PrecompHeader = "espch.h";
            conf.PrecompSource = "espch.cpp";

            conf.AddPublicDependency<LogModule>(target);
            conf.AddPublicDependency<JobSystemModule>(target);
            conf.AddPublicDependency<AssetSystemModule>(target);

            conf.IncludePaths.Add(Path.Combine(Globals.ThirdPartyDirectory, "entt\\include"));

        }
    }
}
