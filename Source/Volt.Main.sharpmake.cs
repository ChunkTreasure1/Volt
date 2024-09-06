
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
	public static class Globals
    {
        // branch root path relative to current sharpmake file location
        public const string RelativeRootPath = @"..\Source";
        public static string RootDirectory;
        
        public const string RelativeThirdPartyPath = @"..\Source\ThirdParty";
        public static string ThirdPartyDirectory;

        public const string RelativeEnginePath = @"..\Engine";
        public static string EngineDirectory;

        public static string RelativeVtProjectPath = @"..\Project\Project.vtproj";
        public static string VtProjectDirectory;
		public static bool ProjectFilepathUpdated = false;

        public const string RelativeSharpmakePath = @"..\Sharpmake";
        public static string SharpmakeDirectory;

        public static string TmpDirectory { get { return Path.Combine(RootDirectory, "../Temp"); } }
        public static string OutputDirectory { get { return Path.Combine(TmpDirectory, "bin"); } }

		[CommandLine.Option("project",
		@"Specify the project to link with the solution: ex: /project('filepath/to/project')")]
		public static void CommandLineProject(string projectArg)
		{
			if (projectArg != "")
			{
				RelativeVtProjectPath = projectArg;
				Environment.SetEnvironmentVariable("VOLT_PROJECT", projectArg);
				ProjectFilepathUpdated = true;
			}
			else
			{
				string voltProjPath = Environment.GetEnvironmentVariable("VOLT_PROJECT");
				if (voltProjPath != null)
				{
					if (voltProjPath != "")
					{
						RelativeVtProjectPath = voltProjPath;
					}
				}
			}
		}
	}

	public static class Main
    {
        private static void ConfigureRootDirectory()
        {
            FileInfo fileInfo = Util.GetCurrentSharpmakeFileInfo();

            string rootDirectory = Path.Combine(fileInfo.DirectoryName, Globals.RelativeRootPath);
            Globals.RootDirectory = Util.SimplifyPath(rootDirectory);

            string thirdPartyDirectory = Path.Combine(fileInfo.DirectoryName, Globals.RelativeThirdPartyPath);
            Globals.ThirdPartyDirectory = Util.SimplifyPath(thirdPartyDirectory);

            string engineDirectory = Path.Combine(fileInfo.DirectoryName, Globals.RelativeEnginePath);
            Globals.EngineDirectory = Util.SimplifyPath(engineDirectory);

            string vtDirectory = Path.Combine(fileInfo.DirectoryName, Globals.RelativeVtProjectPath);
            Globals.VtProjectDirectory = Util.SimplifyPath(vtDirectory);

            string sharpmakeDirectory = Path.Combine(fileInfo.DirectoryName, Globals.RelativeSharpmakePath);
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

			ConfigureRootDirectory();
            ConfigureAutoCleanup();
			HandleProjectFilepathUpdate();

            KitsRootPaths.SetKitsRoot10ToHighestInstalledVersion(DevEnv.vs2022);

            foreach (Type solutionType in Assembly.GetExecutingAssembly().GetTypes().Where(t => !t.IsAbstract && t.IsSubclassOf(typeof(CommonSolution))))
                arguments.Generate(solutionType);
        }
    }
}
