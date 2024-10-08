using Sharpmake;
using System;
using System.IO;
using System.Runtime.CompilerServices;

namespace VoltSharpmake
{
    [Sharpmake.Generate]
    public class VoltAssets : CommonVoltDllProject
	{
        public VoltAssets()
        {
            Name = "Volt-Assets";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.SolutionFolder = "Engine";

            conf.PrecompHeader = "vtapch.h";
            conf.PrecompSource = "vtapch.cpp";

			conf.AddPublicDependency<AssetSystemModule>(target);
			conf.AddPublicDependency<RenderCoreModule>(target);

			// #TODO_Ivar: These should be private. Will fix after new asset import system.
			conf.AddPublicDependency<msdfgen>(target);
			conf.AddPublicDependency<msdf_atlas_gen>(target);
		}
	}
}
