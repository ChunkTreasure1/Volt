using Sharpmake;
using System;
using System.IO;

namespace VoltSharpmake
{
    [Sharpmake.Export]
    public class FbxSDK : Sharpmake.Project
    {
        public FbxSDK() : base(typeof(CommonTarget))
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "FbxSDK";
        }

        [Configure()]
        public void Configure(Configuration conf, CommonTarget target)
        {
            string libSubFolder = "release";

			if (target.Optimization == Optimization.Debug)
			{
				libSubFolder = "debug";
			}

			string libDir = @"[project.RootPath]\lib\x64\" + libSubFolder;

			conf.Output = Configuration.OutputType.None;
			conf.IncludePaths.Add(@"[project.RootPath]\include");
            conf.LibraryPaths.Add(libDir);

			conf.LibraryFiles.Add(
				"libfbxsdk-md.lib",
				"libxml2-md.lib",
				"zlib-md.lib"
			);

			conf.EventPostBuild.Add("copy /Y" + "\"" + libDir + "\\libfbxsdk.dll\"" + "\"" + Globals.EngineDirectory + "\"");
		}
	}
}
