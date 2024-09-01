using System;
using System.Collections.Generic;
using System.Diagnostics;
using Sharpmake;

namespace Volt
{
    [Fragment, Flags]
    public enum Optimization
    {
        Debug = 1 << 0,
        Release = 1 << 1,
        Dist = 1 << 2
    }

    [DebuggerDisplay("\"{Platform}_{DevEnv}\" {Name}")]
    public class CommonTarget : Sharpmake.ITarget
    {
        public Platform Platform;
        public DevEnv DevEnv;
        public Optimization Optimization;
        public Blob Blob;
        public BuildSystem BuildSystem;
        public Sharpmake.DotNetFramework Framework;

        public CommonTarget() { }

        public CommonTarget(
            Platform platform,
            DevEnv devEnv,
            Optimization optimization,
            Blob blob,
            BuildSystem buildSystem,
            DotNetFramework framework
        )
        {
            Platform = platform;
            DevEnv = devEnv;
            Optimization = optimization;
            Blob = blob;
            BuildSystem = buildSystem;
            Framework = framework;
        }

        public override string Name
        {
            get
            {
                var nameParts = new List<string>
                {
                    Optimization.ToString()
                };
                return string.Join(" ", nameParts);
            }
        }

        public string SolutionPlatformName
        {
            get
            {
                var nameParts = new List<string>();

                nameParts.Add(Platform.ToString());

                return string.Join("_", nameParts);
            }
        }

        /// <summary>
        /// returns a string usable as a directory name, to use for instance for the intermediate path
        /// </summary>
        public string DirectoryName
        {
            get
            {
                var dirNameParts = new List<string>();

                dirNameParts.Add(Platform.ToString());
                dirNameParts.Add(Optimization.ToString());
                dirNameParts.Add(BuildSystem.ToString());

                return string.Join("_", dirNameParts);
            }
        }

        public override Sharpmake.Optimization GetOptimization()
        {
            switch (Optimization)
            {
                case Optimization.Debug:
                    return Sharpmake.Optimization.Debug;
                case Optimization.Release:
                    return Sharpmake.Optimization.Release;
                case Optimization.Dist:
                    return Sharpmake.Optimization.Retail;
                default:
                    throw new NotSupportedException("Optimization value " + Optimization.ToString());
            }
        }

        public override Platform GetPlatform()
        {
            return Platform;
        }

        public static CommonTarget[] GetDefaultTargets()
        {
            var result = new List<CommonTarget>();
            result.AddRange(GetWin64Targets());
            return result.ToArray();
        }

        public static CommonTarget[] GetWin64Targets()
        {
            var defaultTarget = new CommonTarget(
                Platform.win64,
                DevEnv.vs2022,
                Optimization.Debug | Optimization.Release | Optimization.Dist,
                Blob.NoBlob,
                BuildSystem.MSBuild,
                DotNetFramework.net6_0
            );

            return new[] { defaultTarget };
        }
    }
}
