using Sharpmake;
using System;
using System.IO;

namespace VoltSharpmake
{
    [Sharpmake.Export]
    public class Steam : Sharpmake.Project
    {
        public Steam() : base(typeof(CommonTarget))
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "Steam";
        }

        [Configure()]
        public void Configure(Configuration conf, CommonTarget target)
        {
            conf.IncludePaths.Add(@"[project.RootPath]\include");
            conf.TargetFileName = @"steam_api64";

            conf.TargetPath = Path.Combine(@"[project.RootPath]\lib\", "win64");
            conf.TargetLibraryPath = Path.Combine(@"[project.RootPath]\lib\", "win64");
            conf.Output = Configuration.OutputType.Dll;
        }
    }
}
