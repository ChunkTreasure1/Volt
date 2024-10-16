using System;
using System.IO;

namespace VoltSharpmake
{
    [Sharpmake.Generate]
    public class VoltCore : CommonVoltDllProject
    {
        public VoltCore() 
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "Volt-Core";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.SolutionFolder = "Engine";

            conf.PrecompHeader = "vtcorepch.h";
            conf.PrecompSource = "vtcorepch.cpp";

			conf.AddPrivateDependency<yaml>(target);

			conf.AddPublicDependency<LogModule>(target);
			conf.AddPublicDependency<EventSystemModule>(target);
			conf.AddPublicDependency<AssetSystemModule>(target);
			conf.AddPublicDependency<EntitySystemModule>(target);
			conf.AddPublicDependency<WindowModule>(target);
			conf.AddPublicDependency<RHIModule>(target);
		}
    }
}
