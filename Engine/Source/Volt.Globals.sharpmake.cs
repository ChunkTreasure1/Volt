using Sharpmake;
using System.IO;
using System;
using System.ComponentModel.Design;

namespace VoltSharpmake
{
	public static class Globals
	{
		public static string SolutionPath;
		public static string RootDirectory;
		public static string GameRootDirectory;
		public static string OutputRootDirectory;
		public static string ThirdPartyDirectory;
		public static string EngineDirectory;
		public static string SharpmakeDirectory;
		public static string PluginsDirectory;

		public static string RelativeVtProjectPath = @"..\..\Project\Project.vtproj";
		public static string VtProjectDirectory;
		public static bool ProjectFilepathUpdated = false;
		public static bool ShouldBuildEngine = true;

		public static string EngineTempDirectory { get { return Path.Combine(RootDirectory, "../Intermediate"); } }
		public static string EngineOutputDirectory { get { return Path.Combine(EngineTempDirectory, "bin"); } }
		
		public static string TempDirectory { get { return Path.Combine(OutputRootDirectory, "../Intermediate"); } }
		public static string OutputDirectory { get { return Path.Combine(TempDirectory, "bin"); } }

		public static string BinariesDirectory { get { return Path.Combine(OutputRootDirectory, "../Binaries"); } }

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
}
