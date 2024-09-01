using System;
using System.IO;

namespace Volt
{
    [Sharpmake.Generate]
    public class tracy : CommonThirdPartyDllProject
    {
        public tracy() : base()
        {
            Name = "tracy";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.ExportDefines.Add("TRACY_ENABLE");
            conf.ExportDefines.Add("TRACY_ON_DEMAND");

            conf.Defines.Add("TRACY_ENABLE");
            conf.Defines.Add("TRACY_ON_DEMAND");
            conf.Defines.Add("TRACY_EXPORTS");

            SourceFilesFilters = new Sharpmake.Strings("public/TracyClient.cpp"); // Add specific .cpp file

            conf.IncludePaths.Add("public");
            conf.IncludePaths.Add("public/tracy");
        }
    }
}
