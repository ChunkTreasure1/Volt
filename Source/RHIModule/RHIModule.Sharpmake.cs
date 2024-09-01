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

            conf.IncludePrivatePaths.Add(Path.Combine(Globals.ThirdPartyDirectory, "Aftermath/include"));
            conf.IncludePrivatePaths.Add(Path.Combine(Globals.ThirdPartyDirectory, "imgui-notify"));
            conf.LibraryPaths.Add(Path.Combine(Globals.ThirdPartyDirectory, "Aftermath/lib/x64"));
            conf.LibraryFiles.Add("GFSDK_Aftermath_Lib.x64.lib");

            conf.Defines.Add("VT_ENABLE_NV_AFTERMATH");
            conf.ExportDefines.Add("VT_ENABLE_NV_AFTERMATH");
        }
    }
}
