using System;
using System.IO;

namespace Volt
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

            conf.IncludePaths.Add(@"[project.RootPath]");
        }
    }
}
