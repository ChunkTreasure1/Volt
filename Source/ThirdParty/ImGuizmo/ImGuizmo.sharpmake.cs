using System;
using System.IO;

namespace Volt
{
    [Sharpmake.Generate]
    public class ImGuizmo : CommonThirdPartyLibProject
    {
        public ImGuizmo()
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "ImGuizmo";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.AddPrivateDependency<imgui>(target);
            conf.IncludePaths.Add(@"[project.RootPath]");
        }
    }
}
