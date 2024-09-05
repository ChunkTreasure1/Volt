using System;
using System.IO;

namespace VoltSharpmake
{
    [Sharpmake.Generate]
    public class EventSystemModule : CommonVoltDllProject
    {
        public EventSystemModule()
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "EventSystemModule";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.SolutionFolder = "Engine";

            conf.PrecompHeader = "eventpch.h";
            conf.PrecompSource = "eventpch.cpp";
        }
    }
}
