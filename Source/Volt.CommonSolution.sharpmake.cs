using Sharpmake;
using System.IO;

namespace VoltSharpmake
{
    public class CommonSolution : Sharpmake.Solution
    {
        public CommonSolution()
            : base(typeof(CommonTarget))
        {
            IsFileNameToLower = false;
        }

        [ConfigurePriority(ConfigurePriorities.All)]
        [Configure]
        public virtual void ConfigureAll(Configuration conf, CommonTarget target)
        {
            conf.SolutionFileName = "[solution.Name]";

            conf.PlatformName = "[target.SolutionPlatformName]";

            conf.SolutionPath = Path.Combine(Globals.RootDirectory, "../");

        }
    }
}
