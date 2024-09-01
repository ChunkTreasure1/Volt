using Sharpmake;
using System.Reflection;
using System;

namespace Volt
{
    [Sharpmake.Generate]
    public class VoltSolution : CommonSolution
    {
        public VoltSolution()
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "Volt";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            //Sharpmake project
            conf.AddProject<SharpmakeProject>(target);

            //Engine
            conf.AddProject<Volt>(target);
            conf.AddProject<CoreUtilities>(target);
            conf.AddProject<LogModule>(target);

            //Editor
            conf.AddProject<Sandbox>(target);

            //ThirdParty
            conf.AddProject<glm>(target);
            conf.AddProject<nfd_extended>(target);
            conf.AddProject<tracy>(target);
            conf.AddProject<yaml>(target);
        }
    }
}
