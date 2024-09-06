using System;
using System.IO;

namespace VoltSharpmake
{
    [Sharpmake.Generate]
    public class AssetSystemModule : CommonVoltDllProject
    {
        public AssetSystemModule()
        {
            Name = "AssetSystemModule";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.SolutionFolder = "Engine";

            conf.PrecompHeader = "aspch.h";
            conf.PrecompSource = "aspch.cpp";

            conf.AddPublicDependency<LogModule>(target);
            conf.AddPublicDependency<JobSystemModule>(target);
        }
    }
}
