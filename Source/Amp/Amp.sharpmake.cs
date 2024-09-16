using System;
using System.IO;

namespace VoltSharpmake
{
    [Sharpmake.Generate]
    public class Amp : CommonVoltLibProject
    {
        public Amp()
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "Amp";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.SolutionFolder = "Engine";

            conf.PrecompHeader = "amppch.h";
            conf.PrecompSource = "amppch.cpp";

            conf.AddPublicDependency<LogModule>(target);

            conf.AddPublicDependency<wwise>(target);
            conf.AddPublicDependency<fmod>(target);
        }
    }
}
