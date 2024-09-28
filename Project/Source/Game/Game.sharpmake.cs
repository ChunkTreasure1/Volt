using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Threading.Tasks;
using Sharpmake;

namespace VoltSharpmake
{
    [Sharpmake.Generate]
    public class Game : CommonVoltPluginProject
    {
        public Game()
        {
            RootPath = Globals.GameRootDirectory;
            SourceRootPath = @"[project.RootPath]/[project.Name]";

            Name = "Game";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            ShouldCopyOnBuild = false;

            base.ConfigureAll(conf, target);

            conf.SolutionFolder = "Game";
            conf.IsExcludedFromBuild = false;
            conf.IntermediatePath = Path.Combine(Globals.GameTempDirectory, @"obj\[target.DirectoryName]\[project.Name]");
            conf.TargetPath = Util.SimplifyPath(Path.Combine(Globals.GameOutputDirectory, @"[target.DirectoryName]\[project.Name]"));
            conf.TargetLibraryPath = Util.SimplifyPath(Path.Combine(Globals.GameOutputDirectory, @"[target.DirectoryName]\[project.Name]"));
            conf.ProjectPath = Path.Combine(Globals.GameRootDirectory, @"[project.Name]");

            conf.EventPostBuild.Add(@"copy /Y " + "\"" + conf.TargetPath + "\\" + Name + ".dll\"" + " \"" + Path.Combine(Globals.VtProjectDirectory, "Plugins") + "\"");

            conf.AddPrivateDependency<Volt>(target);
        }
    }
}