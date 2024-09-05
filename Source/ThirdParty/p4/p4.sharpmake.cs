using Sharpmake;
using System;
using System.IO;

namespace VoltSharpmake
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

            conf.LibraryFiles.Add(
               "libclient.lib",
               "libp4api.lib",
               "libp4script.lib",
               "libp4script_c.lib",
               "libp4script_curl.lib",
               "libp4script_sqlite.lib",
               "librpc.lib",
               "libsupp.lib"
               );

           conf.Output = Configuration.OutputType.None;
        }
    }
}
