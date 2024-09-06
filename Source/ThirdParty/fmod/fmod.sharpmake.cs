using Sharpmake;
using System;
using System.IO;

namespace VoltSharpmake
{
    [Sharpmake.Export]
    public class fmod : Sharpmake.Project
    {
        public fmod() : base(typeof(CommonTarget))
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "fmod";
        }

        [Configure()]
        public void Configure(Configuration conf, CommonTarget target)
        {
            conf.IncludePaths.Add(@"[project.RootPath]\include");
            conf.LibraryPaths.Add(@"[project.RootPath]\lib\" + target.Optimization.ToString());

            conf.Output = Configuration.OutputType.None;
        }
    }
}
