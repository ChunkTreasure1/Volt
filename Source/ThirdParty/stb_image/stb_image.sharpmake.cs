using System;
using System.IO;

namespace Volt
{
    [Sharpmake.Generate]
    public class stb_image : CommonThirdPartyLibProject
    {
        public stb_image()
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "stb_image";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);
            conf.IncludePaths.Add(@"stb");
        }
    }
}
