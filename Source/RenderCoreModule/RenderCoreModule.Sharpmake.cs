using System;
using System.IO;

namespace Volt
{
    [Sharpmake.Generate]
    public class RenderCoreModule : CommonVoltDllProject
    {
        public RenderCoreModule() 
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "RenderCoreModule";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.SolutionFolder = "Engine";

            conf.PrecompHeader = "rcpch.h";
            conf.PrecompSource = "rcpch.cpp";

            conf.AddPublicDependency<LogModule>(target);
            conf.AddPublicDependency<RHIModule>(target);
            conf.AddPublicDependency<JobSystemModule>(target);

            conf.IncludePaths.Add(Path.Combine(Globals.ThirdPartyDirectory, "half"));
        }
    }
}
