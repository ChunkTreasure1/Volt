using System;
using System.IO;

namespace VoltSharpmake
{
    [Sharpmake.Generate]
    public class VoltRenderCore : CommonVoltDllProject
    {
        public VoltRenderCore() 
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "Volt-RenderCore";
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

			conf.AddPrivateDependency<VoltCore>(target);

            conf.IncludePaths.Add(Path.Combine(Globals.ThirdPartyDirectory, "half"));
        }

        public override void ConfigureClangCl(Configuration conf, CommonTarget target)
        {
            base.ConfigureClangCl(conf, target);

            conf.AdditionalCompilerOptions.Add(
                "-Wno-switch"
            );
        }
    }
}
