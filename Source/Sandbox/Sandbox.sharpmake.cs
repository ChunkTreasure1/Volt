using System;
using System.IO;

namespace Volt
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



            conf.AddPublicDependency<ImGuizmo>(target);
            conf.AddPublicDependency<imgui_node_editor>(target);
            conf.AddPublicDependency<p4>(target);
            conf.AddPublicDependency<OpenSSL>(target);

            conf.IncludePaths.Add(
                Path.Combine(Globals.ThirdPartyDirectory ,@"nlohmann/include"),
                Path.Combine(Globals.ThirdPartyDirectory ,@"cpp-httplib/include")
                );

            conf.AdditionalDebuggerCommands = Path.Combine(Globals.VtProjectDirectory, @"Project.vtproj");

            Console.WriteLine(conf.AdditionalDebuggerCommands);

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

            //we have to add the d3d12 to the exe folder
            string d3d12FolderPath = Path.Combine(Globals.EngineDirectory, "D3D12");
            conf.EventPostBuild.Add(@"copy /Y " + "\"" + d3d12FolderPath + "\\D3D12Core.dll\"" + " \"" + conf.TargetPath + "\"");
            conf.EventPostBuild.Add(@"copy /Y " + "\"" + d3d12FolderPath + "\\D3D12Core.pdb\"" + " \"" + conf.TargetPath + "\"");
            conf.EventPostBuild.Add(@"copy /Y " + "\"" + d3d12FolderPath + "\\d3d12SDKLayers.dll\"" + " \"" + conf.TargetPath + "\"");
            conf.EventPostBuild.Add(@"copy /Y " + "\"" + d3d12FolderPath + "\\d3d12SDKLayers.pdb\"" + " \"" + conf.TargetPath + "\"");
        }
    }
}
