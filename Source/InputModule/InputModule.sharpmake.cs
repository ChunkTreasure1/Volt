using System;
using System.IO;

namespace Volt
{
    [Sharpmake.Generate]
    public class InputModule : CommonVoltDllProject
    {
        public InputModule()
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "InputModule";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.SolutionFolder = "Engine";

            conf.PrecompHeader = "inputpch.h";
            conf.PrecompSource = "inputpch.cpp";

            conf.AddPrivateDependency<EventSystemModule>(target);
        }
    }
}