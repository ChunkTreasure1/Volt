using Sharpmake;
using System.Reflection;
using System;
using System.Linq;
using System.IO;

namespace VoltSharpmake
{ 
    [Sharpmake.Generate]
    public class GameSolution : CommonSolution
    {
        public GameSolution()
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "Game";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);
            
            foreach (Type projectType in Assembly.GetExecutingAssembly().GetTypes().Where(t => !t.IsAbstract && t.IsSubclassOf(typeof(CommonProject))))
			{
				conf.AddProject(projectType, target);
			}

			conf.Solution.ExtraItems["Solution Items"] = new Strings(Path.Combine(Globals.GameRootDirectory, ".editorconfig"));
            conf.SetStartupProject<Sandbox>();
		}
	}
}
