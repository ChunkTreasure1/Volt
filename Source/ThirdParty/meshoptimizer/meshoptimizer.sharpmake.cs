using System;
using System.IO;

namespace Volt
{
    [Sharpmake.Generate]
    public class meshoptimizer : CommonThirdPartyLibProject
    {
        public meshoptimizer()
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "meshoptimizer";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.IncludePaths.Add("src");
        }
    }
}
