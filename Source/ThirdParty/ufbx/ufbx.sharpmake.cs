using System;
using System.IO;

namespace VoltSharpmake
{
    [Sharpmake.Generate]
    public class ufbx : CommonThirdPartyLibProject
    {
        public ufbx()
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "ufbx";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.IncludePaths.Add(@"[project.RootPath]/[project.Name]");
        }
    }
}
