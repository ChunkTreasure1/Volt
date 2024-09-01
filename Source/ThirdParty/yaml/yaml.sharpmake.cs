using System;
using System.IO;

namespace Volt
{
    [Sharpmake.Generate]
    public class yaml : CommonThirdPartyLibProject
    {
        public yaml()
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "yaml";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.ExportDefines.Add("TRACY_ENABLE");
            conf.ExportDefines.Add("TRACY_ON_DEMAND");
            conf.ExportDefines.Add("TRACY_EXPORTS");

            conf.IncludePaths.Add("src");
            conf.IncludePaths.Add("include");
        }
    }
}
