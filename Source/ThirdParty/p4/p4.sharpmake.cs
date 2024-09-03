using Sharpmake;
using System;
using System.IO;

namespace Volt
{
    [Sharpmake.Export]
    public class p4 : Sharpmake.Project
    {
        public p4() : base(typeof(CommonTarget))
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "p4";
        }

        [Configure()]
        public void Configure(Configuration conf, CommonTarget target)
        {
            conf.IncludePaths.Add(@"[project.RootPath]\include");
            conf.LibraryPaths.Add(@"[project.RootPath]\lib\" + target.Optimization.ToString());

            conf.Output = Configuration.OutputType.Dll;
        }
    }
}
