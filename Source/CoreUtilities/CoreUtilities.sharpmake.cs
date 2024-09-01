using System;
using System.IO;

namespace Volt
{
    [Sharpmake.Generate]
    public class CoreUtilities : CommonVoltDllProject
    {
        public CoreUtilities() 
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "CoreUtilities";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.SolutionFolder = "Engine";

            conf.PrecompHeader = "cupch.h";
            conf.PrecompSource = "cupch.cpp";

            conf.AddPublicDependency<glm>(target);
            conf.AddPublicDependency<tracy>(target);
            conf.IncludePaths.Add(Path.Combine(Globals.ThirdPartyDirectory, "unordered_dense\\include"));

            conf.AddPrivateDependency<nfd_extended>(target);
            conf.AddPrivateDependency<yaml>(target);
            conf.IncludePrivatePaths.Add(Path.Combine(Globals.ThirdPartyDirectory, "zlib\\include"));
            
            string targetOptimization = target.Optimization.ToString();
            conf.LibraryFiles.Add(Path.Combine(Globals.ThirdPartyDirectory, "zlib\\lib\\" + targetOptimization) + "\\libz-static.lib");
        }
    }
}
