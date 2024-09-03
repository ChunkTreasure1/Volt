//using System;
//using System.IO;

//namespace Volt
//{
//    [Sharpmake.Generate]
//    public class cityhash : CommonThirdPartyLibProject
//    {
//        public cityhash() : base()
//        {
//            SourceRootPath = @"[project.RootPath]/[project.Name]/src";

//            Name = "cityhash";
//        }

//        public override void ConfigureAll(Configuration conf, CommonTarget target)
//        {
//            base.ConfigureAll(conf, target);

//            conf.ExportDefines.Add("GLM_FORCE_DEPTH_ZERO_TO_ONE");
//            conf.ExportDefines.Add("GLM_FORCE_LEFT_HANDED");

//            conf.IncludePaths.Add("../");
//        }
//    }
//}
