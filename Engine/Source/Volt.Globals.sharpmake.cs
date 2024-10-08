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
		public static string ProjectTargetDirectory;

		public static string RelativeVtProjectPath = @"..\..\Project\";
		public static string VtProjectDirectory;
		public static string VtProjectFilePath;
		public static bool ProjectFilepathUpdated = false;
		public static bool ShouldBuildEngine = true;

		public static string EngineTempDirectory { get { return Path.Combine(RootDirectory, "../Intermediate"); } }
		public static string EngineOutputDirectory { get { return Path.Combine(EngineTempDirectory, "bin"); } }
		
		public static string GameTempDirectory { get { return Path.Combine(GameRootDirectory, "../Intermediate"); } }
		public static string GameOutputDirectory { get { return Path.Combine(GameTempDirectory, "bin"); } }

		public static string BinariesDirectory { get { return Path.Combine(RootDirectory, "../Binaries"); } }
	}
}
