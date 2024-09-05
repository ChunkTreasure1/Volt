using System;
using System.IO;

namespace Volt
{
    [Sharpmake.Generate]
    public class imgui_node_editor : CommonThirdPartyLibProject
    {
        public imgui_node_editor()
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "imgui-node-editor";
            SourceRootPath = @"[project.RootPath]";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            SourceFilesFilters = new Sharpmake.Strings(
                "crude_json.cpp",
                "crude_json.h",
                "imgui_bezier_math.h",
                "imgui_bezier_math.inl",
                "imgui_canvas.cpp",
                "imgui_canvas.h",
                "imgui_extra_math.h",
                "imgui_extra_math.inl",
                "imgui_node_editor.cpp",
                "imgui_node_editor.h",
                "imgui_node_editor_api.cpp",
                "imgui_node_edtior_internal.h",
                "imgui_node_edtior_internal.inl",
                "builders.cpp",
                "builders.h",
                "misc/imgui_node_editor.natvis"
            );

            conf.AddPrivateDependency<imgui>(target);

            conf.IncludePaths.Add(@"[project.RootPath]\[project.Name]");
        }
    }
}
