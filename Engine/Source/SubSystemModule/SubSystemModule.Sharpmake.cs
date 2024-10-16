using System;
using System.IO;

namespace VoltSharpmake
{
    [Sharpmake.Generate]
    public class SubSystemModule : CommonVoltDllProject
    {
        public SubSystemModule() 
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "SubSystemModule";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.SolutionFolder = "Engine";

            conf.PrecompHeader = "sspch.h";
            conf.PrecompSource = "sspch.cpp";
		}
    }
}
