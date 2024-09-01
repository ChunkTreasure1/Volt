using Sharpmake;
using System;
using System.IO;

namespace Volt
{
    [Sharpmake.Generate]
    public class SharpmakeProject : Sharpmake.CSharpProject
    {
        public SharpmakeProject()
            : base(typeof(CommonTarget))
        {
            AddTargets(CommonTarget.GetDefaultTargets());

            RootPath = Globals.RootDirectory;

            // This Path will be used to get all SourceFiles in this Folder and all subFolders
            SourceRootPath = Globals.RootDirectory;
        }

        [ConfigurePriority(ConfigurePriorities.All)]
        [Configure]
        public virtual void ConfigureAll(Configuration conf, CommonTarget target)
        {
            conf.Output = Configuration.OutputType.DotNetConsoleApp;

            conf.ProjectFileName = "[project.Name]";
            // Sets where the project file (csproj) will be saved
            conf.ProjectPath = @"[project.RootPath]\[project.Name]";

            conf.ReferencesByPath.Add(Globals.SharpmakeDirectory + "\\Basic.Reference.Assemblies.Net60.dll");
            conf.ReferencesByPath.Add(Globals.SharpmakeDirectory + "\\Microsoft.CodeAnalysis.CSharp.dll");
            conf.ReferencesByPath.Add(Globals.SharpmakeDirectory + "\\Microsoft.CodeAnalysis.dll");
            conf.ReferencesByPath.Add(Globals.SharpmakeDirectory + "\\Microsoft.VisualStudio.Setup.Configuration.Interop.dll");
            conf.ReferencesByPath.Add(Globals.SharpmakeDirectory + "\\Sharpmake.Application.dll");
            conf.ReferencesByPath.Add(Globals.SharpmakeDirectory + "\\Sharpmake.CommonPlatforms.dll");
            conf.ReferencesByPath.Add(Globals.SharpmakeDirectory + "\\Sharpmake.dll");
            conf.ReferencesByPath.Add(Globals.SharpmakeDirectory + "\\Sharpmake.Generators.dll");
            conf.ReferencesByPath.Add(Globals.SharpmakeDirectory + "\\System.Collections.Immutable.dll");
            conf.ReferencesByPath.Add(Globals.SharpmakeDirectory + "\\System.Reflection.Metadata.dll");
            conf.ReferencesByPath.Add(Globals.SharpmakeDirectory + "\\System.Text.Encoding.CodePages.dll");
        }
    };
}