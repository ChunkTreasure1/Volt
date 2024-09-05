using Sharpmake;
using System;
using System.IO;

namespace Volt
{
    [Sharpmake.Export]
    public class PhysX : Sharpmake.Project
    {
        public PhysX() : base(typeof(CommonTarget))
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "PhysX";
        }

        [Configure()]
        public void Configure(Configuration conf, CommonTarget target)
        {
            conf.IncludePaths.Add(@"[project.RootPath]\include");
            conf.TargetFileName = @"PhysX";

            string targetOptimization = target.Optimization.ToString();

            conf.TargetPath = Path.Combine(@"[project.RootPath]\lib\", targetOptimization);
            conf.TargetLibraryPath = Path.Combine(@"[project.RootPath]\lib\", targetOptimization);
            conf.Output = Configuration.OutputType.Lib;
        }
    }
}