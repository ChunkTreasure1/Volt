using Sharpmake;
using System;

namespace Volt
{
    [Sharpmake.Generate]
    public class Volt : CommonVoltLibProject
    {
        public Volt()
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "Volt";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.Output = Configuration.OutputType.Lib;

            conf.SolutionFolder = "Engine";

            // intentionally in a subfolder
            conf.PrecompHeader = "vtpch.h";
            conf.PrecompSource = "vtpch.cpp";

            conf.IncludePaths.Add(SourceRootPath);
        }
    }
}
