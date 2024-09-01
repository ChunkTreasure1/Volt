using System;
using System.IO;

namespace Volt
{
    [Sharpmake.Generate]
    public class glm : CommonThirdPartyLibProject
    {
        public glm() : base()
        {
            SourceRootPath = @"[project.RootPath]/[project.Name]/glm";

            Name = "glm";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.ExportDefines.Add("GLM_FORCE_DEPTH_ZERO_TO_ONE");
            conf.ExportDefines.Add("GLM_FORCE_LEFT_HANDED");

            conf.IncludePaths.Add("../");
        }
    }
}
