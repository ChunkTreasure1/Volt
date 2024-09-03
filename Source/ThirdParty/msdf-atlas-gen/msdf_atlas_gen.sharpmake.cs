using System;
using System.IO;

namespace Volt
{
    [Sharpmake.Generate]
    public class msdf_atlas_gen : CommonThirdPartyLibProject
    {
        public msdf_atlas_gen() : base()
        {
            SourceRootPath = @"[project.RootPath]/[project.Name]/msdf-atlas-gen";

            Name = "msdf-atlas-gen";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.AddPublicDependency<msdfgen>(target);

            conf.IncludePaths.Add(
                "../msdf-atlas-gen",
                "../msdfgen",
                "../msdfgen/include");

            conf.Defines.Add("_CRT_SECURE_NO_WARNINGS");
        }
    }
}
