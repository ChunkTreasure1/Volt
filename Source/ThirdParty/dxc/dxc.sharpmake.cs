using Sharpmake;
using System;
using System.IO;

namespace VoltSharpmake
{
    [Sharpmake.Export]
    public class DXC : Sharpmake.Project
    {
        public DXC() : base(typeof(CommonTarget))
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "DXC";
        }

        [Configure()]
        public void Configure(Configuration conf, CommonTarget target)
        {
            conf.IncludePaths.Add(@"[project.RootPath]\include");
            conf.TargetFileName = @"dxcompiler";
            conf.TargetPath = @"[project.RootPath]\lib";
            conf.TargetLibraryPath = @"[project.RootPath]\lib";
            conf.Output = Configuration.OutputType.Dll;
        }
    }
}
