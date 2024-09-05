using System;
using System.IO;

namespace VoltSharpmake
{
    [Sharpmake.Generate]
    public class msdfgen : CommonThirdPartyLibProject
    {
        public msdfgen() : base()
        {
            SourceRootPath = @"[project.RootPath]/msdf-atlas-gen/msdfgen/core";

            AdditionalSourceRootPaths.Add(
                @"[project.RootPath]/msdf-atlas-gen/msdfgen/ext",
                @"[project.RootPath]/msdf-atlas-gen/msdfgen/lib",
                @"[project.RootPath]/msdf-atlas-gen/msdfgen/include"
                );

            Name = "msdfgen";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.AddPublicDependency<freetype>(target);

            conf.IncludePaths.Add("../ext");
            conf.IncludePaths.Add("../lib");
            conf.IncludePaths.Add("../include");

            conf.Defines.Add("_CRT_SECURE_NO_WARNINGS");
        }
    }
}
