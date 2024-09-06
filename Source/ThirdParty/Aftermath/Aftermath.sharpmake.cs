using Sharpmake;
using System;
using System.IO;

namespace VoltSharpmake
{
    [Sharpmake.Export]
    public class Aftermath : Sharpmake.Project
    {
        public Aftermath() : base(typeof(CommonTarget))
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "Aftermath";
        }

        [Configure()]
        public void Configure(Configuration conf, CommonTarget target)
        {
            conf.IncludePaths.Add(@"[project.RootPath]\include");
            conf.TargetFileName = @"GFSDK_Aftermath_Lib.x64";
            conf.TargetPath = @"[project.RootPath]\lib\x64";
            conf.TargetLibraryPath = @"[project.RootPath]\lib\x64";
            conf.Output = Configuration.OutputType.Dll;
        }
    }
}
