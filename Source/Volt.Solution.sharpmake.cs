using Sharpmake;
using System.Reflection;
using System;
using System.Linq;

namespace Volt
{ 
    [Sharpmake.Generate]
    public class VoltSolution : CommonSolution
    {
        public VoltSolution()
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "Volt";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            //Sharpmake project, special case since it isnt a CommonProject
            conf.AddProject<SharpmakeProject>(target);

            foreach (Type projectType in Assembly.GetExecutingAssembly().GetTypes().Where(t => !t.IsAbstract && t.IsSubclassOf(typeof(CommonProject))))
                conf.AddProject(projectType, target);
        }
    }
}
