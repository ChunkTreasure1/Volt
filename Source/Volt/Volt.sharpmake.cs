using Sharpmake;
using System;
using System.IO;

namespace Volt
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

            conf.AddPrivateDependency<LogModule>(target);
            conf.AddPrivateDependency<AssetSystemModule>(target);
            conf.AddPrivateDependency<EntitySystemModule>(target);
            conf.AddPrivateDependency<EventSystemModule>(target);
            conf.AddPrivateDependency<RHIModule>(target);
            conf.AddPrivateDependency<RenderCoreModule>(target);
            conf.AddPrivateDependency<NavigationModule>(target);
            conf.AddPrivateDependency<MosaicModule>(target);
            conf.AddPrivateDependency<WindowModule>(target);
            conf.AddPrivateDependency<InputModule>(target);
            conf.AddPrivateDependency<Amp>(target);

            conf.AddPrivateDependency<VulkanRHIModule>(target);
            conf.AddPrivateDependency<D3D12RHIModule>(target);

            conf.AddPrivateDependency<PhysX>(target);
            conf.AddPrivateDependency<meshoptimizer>(target);
            conf.AddPrivateDependency<stb_image>(target);
            conf.AddPrivateDependency<ufbx>(target);
            conf.AddPrivateDependency<msdfgen>(target);
            conf.AddPrivateDependency<msdf_atlas_gen>(target);
            conf.AddPrivateDependency<Steam>(target);
            conf.AddPrivateDependency<libacc>(target);
            conf.AddPrivateDependency<METIS>(target);

            conf.IncludePrivatePaths.Add(Path.Combine(Globals.ThirdPartyDirectory, "tiny_gltf"));
            conf.IncludePrivatePaths.Add(Path.Combine(Globals.ThirdPartyDirectory, "tinyddsloader"));
        }
    }
}
