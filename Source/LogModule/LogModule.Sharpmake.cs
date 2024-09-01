using System;
using System.IO;

namespace Volt
{
    [Sharpmake.Generate]
    public class LogModule : CommonVoltDllProject
    {
        public LogModule() 
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "LogModule";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.SolutionFolder = "Engine";
            conf.IncludePrivatePaths.Add(Path.Combine(Globals.ThirdPartyDirectory, "spdlog/include"));
        }
    }
}
