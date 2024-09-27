
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Threading.Tasks;
using Sharpmake;

[module: Sharpmake.Include("Volt.*.sharpmake.cs")]
[module: Sharpmake.Include("*/*.sharpmake.cs")]
[module: Sharpmake.Include("ThirdParty/*/*.sharpmake.cs")]
[module: Sharpmake.Include("ThirdParty/*/*/*.sharpmake.cs")]

namespace VoltSharpmake
{
	public static class Main
    {
        private static void ConfigureGlobals()
        {
            FileInfo fileInfo = Util.GetCurrentSharpmakeFileInfo();

            Globals.RootDirectory = Util.SimplifyPath(fileInfo.DirectoryName);
            Globals.ThirdPartyDirectory = Util.SimplifyPath(Path.Combine(Globals.RootDirectory, "ThirdParty"));
			Globals.SolutionPath = Path.Combine(Globals.RootDirectory, "../../");
			Globals.EngineDirectory = Util.SimplifyPath(Path.Combine(Globals.RootDirectory, "../"));
			Globals.PluginsDirectory = Util.SimplifyPath(Path.Combine(Globals.EngineDirectory, "Plugins"));
            Globals.OutputRootDirectory = Globals.RootDirectory;

            Globals.VtProjectDirectory = Path.Combine(Globals.RootDirectory, "../Project.vtproj");

            string sharpmakeDirectory = Path.Combine(Globals.RootDirectory, "../../Sharpmake");
            Globals.SharpmakeDirectory = Util.SimplifyPath(sharpmakeDirectory);
        }

        private static void ConfigureAutoCleanup()
        {
            Util.FilesAutoCleanupActive = true;
            Util.FilesAutoCleanupDBPath = Path.Combine(Globals.TmpDirectory, "sharpmake");

            if (!Directory.Exists(Util.FilesAutoCleanupDBPath))
                Directory.CreateDirectory(Util.FilesAutoCleanupDBPath);
        }

		private static void HandleProjectFilepathUpdate()
		{
			if (!Globals.ProjectFilepathUpdated) 
			{
				return;
			}

			// Delete all vcxproj, as the command line args won't be updated otherwise...
			string[] files = Directory.GetFiles(Globals.RootDirectory, "*vcxproj*", SearchOption.AllDirectories);
			foreach (string file in files)
			{
				if (File.Exists(file))
				{
					File.Delete(file);
				}
			}
		}

        [Sharpmake.Main]
        public static void SharpmakeMain(Sharpmake.Arguments arguments)
        {
			CommandLine.ExecuteOnType(typeof(VoltSharpmake.Globals));

			ConfigureGlobals();
            ConfigureAutoCleanup();
			HandleProjectFilepathUpdate();

            KitsRootPaths.SetKitsRoot10ToHighestInstalledVersion(DevEnv.vs2022);

			foreach (Type solutionType in Assembly.GetExecutingAssembly().GetTypes().Where(t => !t.IsAbstract && t.IsSubclassOf(typeof(CommonSolution))))
                arguments.Generate(solutionType);
        }
    }
}
