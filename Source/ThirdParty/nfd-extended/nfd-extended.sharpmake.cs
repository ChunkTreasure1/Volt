using System;
using System.IO;

namespace Volt
{
    [Sharpmake.Generate]
    public class nfd_extended : CommonThirdPartyLibProject
    {
        public nfd_extended() : base()
        {
            Name = "nfd-extended";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            SourceFilesFilters = new Sharpmake.Strings("src/nfd_win.cpp"); // Add specific .cpp file

            conf.IncludePaths.Add("src/include/");
        }
    }
}
