using System;
using System.IO;

namespace VoltSharpmake
{
    [Sharpmake.Generate]
    public class JobSystemModule : CommonVoltDllProject
    {
        public JobSystemModule()
        {
            Name = "JobSystemModule";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.SolutionFolder = "Engine";

            conf.PrecompHeader = "jspch.h";
            conf.PrecompSource = "jspch.cpp";

            conf.AddPublicDependency<LogModule>(target);
        }
    }
}
