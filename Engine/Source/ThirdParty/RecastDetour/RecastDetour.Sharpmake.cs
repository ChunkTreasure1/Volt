using System;
using System.IO;

namespace VoltSharpmake
{
    [Sharpmake.Generate]
    public class RecastDetour : CommonThirdPartyLibProject
    {
        public RecastDetour()
        {
            Name = "RecastDetour";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.IncludePaths.Add("DebugUtils/Include");
            conf.IncludePaths.Add("Detour/Include");
            conf.IncludePaths.Add("DetourCrowd/Include");
            conf.IncludePaths.Add("DetourTileCache/Include");
            conf.IncludePaths.Add("Fastlz/Include");
            conf.IncludePaths.Add("Recast/Include");
        }
    }
}
