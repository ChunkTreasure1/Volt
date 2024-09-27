using Sharpmake;
using System;
using System.IO;

namespace VoltSharpmake
{
    [Sharpmake.Generate]
    public class Sandbox : CommonVoltExeProject
    {
        public Sandbox()
        {
            Name = "Sandbox";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.SolutionFolder = "";

            conf.PrecompHeader = "sbpch.h";
            conf.PrecompSource = "sbpch.cpp";

            conf.VcxprojUserFile = new Configuration.VcxprojUserFileSettings
            {
                LocalDebuggerWorkingDirectory = Globals.EngineDirectory,
                LocalDebuggerCommandArguments = Globals.VtProjectDirectory
            };

            conf.AddPublicDependency<Volt>(target);

            conf.AddPublicDependency<ImGuizmo>(target);
            conf.AddPublicDependency<imgui_node_editor>(target);
            conf.AddPublicDependency<p4>(target);
            conf.AddPublicDependency<OpenSSL>(target);

            conf.AddPublicDependency<nfd_extended>(target);
            conf.AddPublicDependency<yaml>(target);
            conf.AddPublicDependency<MosaicModule>(target);

            conf.AddPublicDependency<glm>(target);

            conf.AddPrivateDependency<esfw>(target);

            conf.IncludePaths.Add(
                Path.Combine(Globals.ThirdPartyDirectory ,@"nlohmann/include"),
                Path.Combine(Globals.ThirdPartyDirectory ,@"cpp-httplib/include")
                );

            conf.AdditionalDebuggerCommands = Path.Combine(Globals.VtProjectDirectory, @"Project.vtproj");

            conf.Defines.Add("CPPHTTPLIB_OPENSSL_SUPPORT");

        }

        public override void ConfigureWin64(Configuration conf, CommonTarget target)
        {
            base.ConfigureWin64(conf, target);

            conf.LibraryFiles.Add(
                "crypt32.lib",
                "Bcrypt.lib",

                "Winmm.lib",
                "Version.lib"
                );

            // This copy should probably be moved to the D3D12RHIModule script.
            string d3d12FolderPath = Path.Combine(Globals.ThirdPartyDirectory, "d3d12", "Binaries");
            conf.EventPostBuild.Add(@"copy /Y " + "\"" + d3d12FolderPath + "\\D3D12Core.dll\"" + " \"" + conf.TargetPath + "\"");
            conf.EventPostBuild.Add(@"copy /Y " + "\"" + d3d12FolderPath + "\\D3D12Core.pdb\"" + " \"" + conf.TargetPath + "\"");
            conf.EventPostBuild.Add(@"copy /Y " + "\"" + d3d12FolderPath + "\\d3d12SDKLayers.dll\"" + " \"" + conf.TargetPath + "\"");
            conf.EventPostBuild.Add(@"copy /Y " + "\"" + d3d12FolderPath + "\\d3d12SDKLayers.pdb\"" + " \"" + conf.TargetPath + "\"");
        }

        public override void ConfigureMSVC(Configuration conf, CommonTarget target)
        {
            base.ConfigureMSVC(conf, target);
            conf.AdditionalLinkerOptions.Add(
                "/WHOLEARCHIVE:PhysX",
                "/WHOLEARCHIVE:Volt",
                "/WHOLEARCHIVE:MosaicModule"
                );

            conf.Options.Add(new Sharpmake.Options.Vc.Compiler.DisableSpecificWarnings("4098","4217"));
        }
    }
}
