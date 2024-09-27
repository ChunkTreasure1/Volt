using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Threading.Tasks;
using Sharpmake;

[module: Sharpmake.Include("%VOLT_PATH%/Source/Volt.CommonProject.sharpmake.cs")]
[module: Sharpmake.Include("%VOLT_PATH%/Source/Volt.CommonSolution.sharpmake.cs")]
[module: Sharpmake.Include("%VOLT_PATH%/Source/Volt.CommonTarget.sharpmake.cs")]
[module: Sharpmake.Include("%VOLT_PATH%/Source/Volt.Globals.sharpmake.cs")]
[module: Sharpmake.Include("%VOLT_PATH%/Source/*/*.sharpmake.cs")]
[module: Sharpmake.Include("%VOLT_PATH%/Source/ThirdParty/*/*.sharpmake.cs")]
[module: Sharpmake.Include("%VOLT_PATH%/Source/ThirdParty/*/*/*.sharpmake.cs")]
[module: Sharpmake.Include("Game.*.sharpmake.cs")]
[module: Sharpmake.Include("*/*.sharpmake.cs")]

namespace VoltSharpmake
{
	public static class Main
    {
        private static void ConfigureGlobals()
        {
            string absoluteEngineRootPath = Environment.GetEnvironmentVariable("VOLT_PATH");

            FileInfo fileInfo = Util.GetCurrentSharpmakeFileInfo();

            Globals.RootDirectory = Util.SimplifyPath(Path.Combine(absoluteEngineRootPath, "Source"));
            Globals.ThirdPartyDirectory = Util.SimplifyPath(Path.Combine(Globals.RootDirectory, "ThirdParty"));
            Globals.GameRootDirectory = Util.SimplifyPath(fileInfo.DirectoryName);
            Globals.SolutionPath = Globals.GameRootDirectory;
            Globals.EngineDirectory = Util.SimplifyPath(absoluteEngineRootPath);
            Globals.PluginsDirectory = Util.SimplifyPath(Path.Combine(Globals.EngineDirectory, "Plugins"));
            Globals.SharpmakeDirectory = Util.SimplifyPath(Path.Combine(fileInfo.DirectoryName, "Sharpmake"));
            Globals.VtProjectDirectory = Util.SimplifyPath(Path.Combine(fileInfo.DirectoryName, "../"));
            Globals.OutputRootDirectory = Globals.GameRootDirectory;
            Globals.ShouldBuildEngine = false;
        }

        private static void ConfigureAutoCleanup()
        {
            Util.FilesAutoCleanupActive = true;
            Util.FilesAutoCleanupDBPath = Path.Combine(Globals.TempDirectory, "sharpmake");

            if (!Directory.Exists(Util.FilesAutoCleanupDBPath))
                Directory.CreateDirectory(Util.FilesAutoCleanupDBPath);
        }

        [Sharpmake.Main]
        public static void SharpmakeMain(Sharpmake.Arguments arguments)
        {
			ConfigureGlobals();
            ConfigureAutoCleanup();

            KitsRootPaths.SetKitsRoot10ToHighestInstalledVersion(DevEnv.vs2022);

            foreach (Type solutionType in Assembly.GetExecutingAssembly().GetTypes().Where(t => !t.IsAbstract && t.IsSubclassOf(typeof(CommonSolution))))
                arguments.Generate(solutionType);        
		}
    }
}
