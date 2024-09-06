using System;
using System.IO;

namespace VoltSharpmake
{
    [Sharpmake.Generate]
    public class MosaicModule : CommonVoltLibProject
    {
        public MosaicModule()
        {
            Name = "MosaicModule";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.SolutionFolder = "Engine";

            conf.PrecompHeader = "mcpch.h";
            conf.PrecompSource = "mcpch.cpp";

            conf.AddPublicDependency<LogModule>(target);
        }
    }
}
