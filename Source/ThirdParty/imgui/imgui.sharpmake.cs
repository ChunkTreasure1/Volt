using Sharpmake;
using System;
using System.IO;

namespace Volt
{
    [Sharpmake.Generate]
    public class imgui : CommonThirdPartyLibProject
    {
        public imgui() : base()
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "imgui";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            SourceFilesFilters = new Sharpmake.Strings(
                "imgui.h",
                "imgui.cpp",
                "imgui_draw.cpp",
                "imgui_internal.h",
                "imgui_widgets.cpp",
                "imstb_rectpack.h",
                "imstb_textedit.h",
                "imstb_truetype.h",
                "imgui_demo.cpp",
                "imgui_tables.cpp",
                "imgui_stdlib.cpp",
                "imgui_stdlib.h",
                "imgui_bezier.h"
            );

            conf.IncludePaths.Add("[project.RootPath]/[project.Name]");
        }
    }
}
