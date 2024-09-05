using Sharpmake;
using System;
using System.IO;

namespace VoltSharpmake
{
    [Sharpmake.Export]
    public class libacc : Sharpmake.Project
    {
        public libacc() : base(typeof(CommonTarget))
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "libacc";
        }

        [Configure()]
        public void Configure(Configuration conf, CommonTarget target)
        {
            conf.IncludePaths.Add(@"[project.RootPath]");
        }
    }
}
