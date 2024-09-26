using Sharpmake;
using System;
using System.IO;
using System.Runtime.CompilerServices;

namespace VoltSharpmake
{
    [Sharpmake.Generate]
    public class Volt : CommonVoltLibProject
    {
        public Volt()
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "Volt";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.SolutionFolder = "Engine";

            conf.PrecompHeader = "vtpch.h";
            conf.PrecompSource = "vtpch.cpp";

            conf.AddPublicDependency<LogModule>(target);
            conf.AddPublicDependency<AssetSystemModule>(target);
            conf.AddPublicDependency<EntitySystemModule>(target);
            conf.AddPublicDependency<EventSystemModule>(target);
            conf.AddPublicDependency<RHIModule>(target);
            conf.AddPublicDependency<RenderCoreModule>(target);
            conf.AddPublicDependency<WindowModule>(target);
            conf.AddPublicDependency<InputModule>(target);

            conf.AddPublicDependency<VulkanRHIModule>(target);
            conf.AddPublicDependency<D3D12RHIModule>(target);


            conf.AddPublicDependency<NavigationModule>(target);
            conf.AddPublicDependency<MosaicModule>(target);
            conf.AddPublicDependency<Amp>(target);

            conf.AddPublicDependency<PhysX>(target);
            conf.AddPrivateDependency<meshoptimizer>(target);
            conf.AddPublicDependency<stb_image>(target);
            conf.AddPrivateDependency<msdfgen>(target);
            conf.AddPrivateDependency<msdf_atlas_gen>(target);
            conf.AddPrivateDependency<Steam>(target);
            conf.AddPrivateDependency<libacc>(target);
            conf.AddPrivateDependency<METIS>(target);
			conf.AddPrivateDependency<FbxSDK>(target);

            conf.IncludePrivatePaths.Add(Path.Combine(Globals.ThirdPartyDirectory, "tiny_gltf"));
            conf.IncludePrivatePaths.Add(Path.Combine(Globals.ThirdPartyDirectory, "tinyddsloader"));
        }

		public override void ConfigureRelease(Configuration conf, CommonTarget target)
		{
			base.ConfigureRelease(conf, target);

			conf.AdditionalCompilerOptions.Add("/bigobj");
		}

		public override void ConfigureDist(Configuration conf, CommonTarget target)
		{
			base.ConfigureDist(conf, target);

			conf.AdditionalCompilerOptions.Add("/bigobj");
		}
	}
}
