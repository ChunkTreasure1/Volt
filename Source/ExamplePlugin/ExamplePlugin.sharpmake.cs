using System;
using System.IO;

namespace Volt
{
    [Sharpmake.Generate]
    public class ExamplePlugin : CommonVoltPluginProject
    {
        public ExamplePlugin()
        {
            Name = "ExamplePlugin";
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
        }
    }
}