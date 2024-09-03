using Sharpmake;
using System;
using System.IO;

namespace Volt
{
    [Sharpmake.Export]
    public class METIS : Sharpmake.Project
    {
        public METIS() : base(typeof(CommonTarget))
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "METIS";
        }

        [Configure()]
        public void Configure(Configuration conf, CommonTarget target)
        {
            conf.IncludePaths.Add(@"[project.RootPath]\include");
            conf.LibraryPaths.Add(@"[project.RootPath]\libmetis\" + target.Optimization.ToString());

            conf.Output = Configuration.OutputType.Dll;
        }
    }
}
