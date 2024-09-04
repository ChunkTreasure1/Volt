using System;
using System.IO;

namespace Volt
{
    [Sharpmake.Generate]
    public class D3D12RHIModule : CommonVoltDllProject
    {
        public D3D12RHIModule() 
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "D3D12RHIModule";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.SolutionFolder = "Engine/RHI";

            conf.PrecompHeader = "dxpch.h";
            conf.PrecompSource = "dxpch.cpp";

            conf.AddPublicDependency<RHIModule>(target);
            conf.AddPublicDependency<GLFW>(target);
            conf.AddPublicDependency<LogModule>(target);
            conf.AddPublicDependency<imgui>(target);

            conf.AddPrivateDependency<Aftermath>(target);
            conf.AddPrivateDependency<DXC>(target);

            conf.IncludePrivatePaths.Add(Path.Combine(Globals.ThirdPartyDirectory, "d3d12"));

            conf.LibraryFiles.Add("d3d12.lib");
            conf.LibraryFiles.Add("DXGI.lib");
            conf.LibraryFiles.Add("dxguid.lib");
        }

        public override void ConfigureClangCl(Configuration conf, CommonTarget target)
        {
            base.ConfigureClangCl(conf, target);

            conf.AdditionalCompilerOptions.Add(
                "-Wno-switch",
                "-Wno-delete-non-abstract-non-virtual-dtor",
                "-Wno-tautological-undefined-compare",
                "-Wno-unused-const-variable",
                "-Wno-unused-value",
                "-Wno-unused-private-field"
            );
        }
    }
}
