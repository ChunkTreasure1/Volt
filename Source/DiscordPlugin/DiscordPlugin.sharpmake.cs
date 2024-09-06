using System;
using System.IO;

namespace VoltSharpmake
{
    [Sharpmake.Generate]
    public class DiscordPlugin : CommonVoltPluginProject
    {
        public DiscordPlugin()
        {
            Name = "DiscordPlugin";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.SolutionFolder = "Plugins";

            conf.AddPublicDependency<LogModule>(target);
            conf.AddPublicDependency<RHIModule>(target);
            conf.AddPublicDependency<JobSystemModule>(target);
            conf.AddPublicDependency<EventSystemModule>(target);
            conf.AddPublicDependency<Volt>(target);

            conf.AddPublicDependency<DiscordSDK>(target);

        }
    }
}