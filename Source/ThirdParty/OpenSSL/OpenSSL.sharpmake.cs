using Sharpmake;
using System;
using System.IO;

namespace Volt
{
    [Sharpmake.Export]
    public class OpenSSL : Sharpmake.Project
    {
        public OpenSSL() : base(typeof(CommonTarget))
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "OpenSSL";
        }

        [Configure()]
        public void Configure(Configuration conf, CommonTarget target)
        {
            conf.IncludePaths.Add(@"[project.RootPath]\include");
            conf.LibraryPaths.Add(@"[project.RootPath]\lib\" + target.Optimization.ToString());

            conf.LibraryFiles.Add(
                "libcrypto.lib",
                "libssl.lib"
                );
            conf.Output = Configuration.OutputType.None;
        }
    }
}
