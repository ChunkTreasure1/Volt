using Sharpmake;
using System;
using System.IO;

namespace Volt
{
    [Sharpmake.Generate]
    public class PhysX : CommonThirdPartyLibProject
    {
        public PhysX()
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "PhysX";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.Defines.Add(
                "PX_PHYSX_STATIC_LIB",
                "PX_COOKING",
                "WIN64",
                "_CRT_SECURE_NO_DEPRECATE",
                "_CRT_NONSTDC_NO_DEPRECATE",
                "_WINSOCK_DEPRECATED_NO_WARNINGS",
                "DISABLE_CUDA_PHYSX"
            );
            conf.ExportDefines.Add(
                "PX_PHYSX_STATIC_LIB",
                "PX_COOKING"
                );

            conf.IncludePaths.Add(@"[project.RootPath]/[project.Name]/include");

            conf.IncludePrivatePaths.Add(
                "include/PhysX",
                "source/common/include/",
                "source/common/src/",
                "source/physx/src/",
                "source/physx/src/device/",
                "source/physx/src/buffering/",
                "source/physxgpu/include/",
                "source/geomutils/include/",
                "source/geomutils/src/",
                "source/geomutils/src/contact/",
                "source/geomutils/src/common/",
                "source/geomutils/src/convex/",
                "source/geomutils/src/distance/",
                "source/geomutils/src/sweep/",
                "source/geomutils/src/gjk/",
                "source/geomutils/src/intersection/",
                "source/geomutils/src/mesh/",
                "source/geomutils/src/hf/",
                "source/geomutils/src/pcm/",
                "source/geomutils/src/ccd/",
                "source/lowlevel/api/include",
                "source/lowlevel/software/include",
                "source/lowlevel/common/include",
                "source/lowlevel/common/include/pipeline",
                "source/lowlevel/common/include/collision",
                "source/lowlevel/common/include/utils",
                "source/lowlevelaabb/include",
                "source/lowleveldynamics/include",
                "source/simulationcontroller/include",
                "source/simulationcontroller/src",
                "source/physxcooking/src",
                "source/physxcooking/src/mesh",
                "source/physxcooking/src/convex",
                "source/scenequery/include",
                "source/physxmetadata/core/include",
                "source/immediatemode/include",
                "source/pvd/include",
                "source/foundation/include",
                "source/fastxml/include",
                "source/physxextensions/src/serialization/File",
                "source/physxmetadata/extensions/include",
                "source/physxextensions/src/serialization/Xml",
                "source/physxextensions/src/serialization",
                "source/physxextensions/src",
                "source/physxextensions/src/serialization/Binary",
                "source/physxvehicle/src/physxmetadata/include",
                "source/filebuf/include",
                "source/physxvehicle/src",
                "pxshared/include"
            );
        }

        public override void ConfigureDebug(Configuration conf, CommonTarget target)
        {
            base.ConfigureDebug(conf, target);

            conf.Defines.Add(
                "PX_DEBUG=1",
                "PX_CHECKED=1",
                "PX_NVTX=0",
                "PX_SUPPORT_PVD=1"
            );
        }

        public override void ConfigureRelease(Configuration conf, CommonTarget target)
        {
            base.ConfigureRelease(conf, target);

            conf.Defines.Add(
                "PX_PROFILE=1",
                "PX_NVTX=0",
                "PX_SUPPORT_PVD=1",
                "NDEBUG"
            );
        }

        public override void ConfigureDist(Configuration conf, CommonTarget target)
        {
            base.ConfigureDist(conf, target);

            conf.Defines.Add(
                "PX_SUPPORT_PVD=0",
                "NDEBUG"
            );
        }

        public override void ConfigureClangCl(Configuration conf, CommonTarget target)
        {
            base.ConfigureClangCl(conf, target);

            conf.Defines.Add("PX_SIMD_DISABLED=0");
        }
    }
}
