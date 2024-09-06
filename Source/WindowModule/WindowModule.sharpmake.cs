using System;
using System.IO;

namespace VoltSharpmake
{
    [Sharpmake.Generate]
    public class WindowModule : CommonVoltDllProject
    {
        public WindowModule()
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "WindowModule";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.SolutionFolder = "Engine";

            conf.PrecompHeader = "windowpch.h";
            conf.PrecompSource = "windowpch.cpp";

            conf.AddPrivateDependency<GLFW>(target);
            conf.AddPrivateDependency<DirectXTex>(target);
            conf.AddPrivateDependency<InputModule>(target);
            conf.AddPrivateDependency<EventSystemModule>(target);
            conf.AddPrivateDependency<RHIModule>(target);
            conf.AddPrivateDependency<LogModule>(target);
        }
    }
}
