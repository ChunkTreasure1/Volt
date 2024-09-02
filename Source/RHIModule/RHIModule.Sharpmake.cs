using System;
using System.IO;

namespace Volt
{
    [Sharpmake.Generate]
    public class RHIModule : CommonVoltDllProject
    {
        public RHIModule() 
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "RHIModule";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.SolutionFolder = "Engine/RHI";

            conf.PrecompHeader = "rhipch.h";
            conf.PrecompSource = "rhipch.cpp";

            conf.AddPublicDependency<LogModule>(target);
            conf.AddPublicDependency<imgui>(target);
            conf.AddPrivateDependency<Aftermath>(target);

            conf.IncludePaths.Add(Path.Combine(Globals.ThirdPartyDirectory, "imgui-notify"));

            conf.Defines.Add("VT_ENABLE_NV_AFTERMATH");
            conf.ExportDefines.Add("VT_ENABLE_NV_AFTERMATH");
        }
    }
}
