using System;
using System.IO;
using System.Linq;
using Sharpmake;

namespace VoltSharpmake
{
    public static class ConfigurePriorities
    {
        public const int All = -75;
        public const int Platform = -50;
        public const int Optimization = -25;
        /*     SHARPMAKE DEFAULT IS 0     */
        public const int Blobbing = 25;
        public const int BuildSystem = 50;
        public const int Compiler = 75;
    }

    public abstract class CommonProject : Sharpmake.Project
    {
        protected CommonProject()
            : base(typeof(CommonTarget))
        {
            AddTargets(CommonTarget.GetDefaultTargets());

            RootPath = Globals.RootDirectory; // default to RootDirectory
            IsFileNameToLower = false;
            IsTargetFileNameToLower = false;

            SourceRootPath = @"[project.RootPath]/[project.Name]";
        }

        [ConfigurePriority(ConfigurePriorities.All)]
        [Configure]
        public virtual void ConfigureAll(Configuration conf, CommonTarget target)
        {
            conf.ProjectFileName = "[project.Name]_[target.Platform]";
            if (target.DevEnv != DevEnv.xcode)
                conf.ProjectFileName += "_[target.DevEnv]";

            conf.IntermediatePath = Path.Combine(Globals.TmpDirectory, @"obj\[target.DirectoryName]\[project.Name]");

            // intentionally put all of the project outputs in their own directory
            conf.TargetPath = Util.SimplifyPath(Path.Combine(Globals.OutputDirectory, @"[target.DirectoryName]\[project.Name]"));

            conf.TargetLibraryPath = Path.Combine(Globals.OutputDirectory, @"[target.DirectoryName]\[project.Name]");

            conf.Output = Configuration.OutputType.Lib; // defaults to creating static libs

            conf.Options.Add(Options.Vc.Compiler.CppLanguageStandard.CPP20);
            conf.Options.Add(Options.Vc.Compiler.Exceptions.Enable);
            conf.Options.Add(Options.Vc.Compiler.RTTI.Disable);
            conf.Options.Add(Options.Vc.Compiler.FloatingPointModel.Precise);

            conf.Options.Add(Options.Vc.Linker.GenerateMapFile.Disable);

            conf.Options.Add(Options.Vc.General.CharacterSet.Unicode);
        }

        ////////////////////////////////////////////////////////////////////////
        #region Platfoms
        [ConfigurePriority(ConfigurePriorities.Platform)]
        [Configure(Platform.win64)]
        public virtual void ConfigureWin64(Configuration conf, CommonTarget target)
        {
            conf.Defines.Add("_HAS_EXCEPTIONS=0");
            conf.Defines.Add("VT_PLATFORM_WINDOWS");
        }

        [ConfigurePriority(ConfigurePriorities.Platform)]
        [Configure(Platform.linux)]
        public virtual void ConfigureLinux(Configuration conf, CommonTarget target)
        {
        }

        #endregion
        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        #region Optimizations
        [ConfigurePriority(ConfigurePriorities.Optimization)]
        [Configure(Optimization.Debug)]
        public virtual void ConfigureDebug(Configuration conf, CommonTarget target)
        {
            conf.DefaultOption = Options.DefaultTarget.Debug;
            conf.Defines.Add("VT_DEBUG");
            conf.Defines.Add("VT_ENABLE_ASSERTS");
            conf.Defines.Add("VT_ENABLE_VALIDATION");
            conf.Defines.Add("VT_ENABLE_PROFILING");

            conf.Options.Add(Options.Vc.Compiler.RuntimeLibrary.MultiThreadedDebugDLL);
        }

        [ConfigurePriority(ConfigurePriorities.Optimization)]
        [Configure(Optimization.Release)]
        public virtual void ConfigureRelease(Configuration conf, CommonTarget target)
        {
            conf.DefaultOption = Options.DefaultTarget.Release;
            conf.Defines.Add("VT_RELEASE");

            conf.Defines.Add("VT_ENABLE_ASSERTS");
            conf.Defines.Add("VT_ENABLE_VALIDATION");
            conf.Defines.Add("VT_ENABLE_PROFILING");

            conf.Defines.Add("NDEBUG");

            conf.Options.Add(Options.Vc.Compiler.RuntimeLibrary.MultiThreadedDLL);
        }

        [ConfigurePriority(ConfigurePriorities.Optimization)]
        [Configure(Optimization.Dist)]
        public virtual void ConfigureDist(Configuration conf, CommonTarget target)
        {
            conf.DefaultOption = Options.DefaultTarget.Release;
            conf.Defines.Add("VT_DIST");

            conf.Defines.Add("NDEBUG");

            conf.Options.Add(Options.Vc.Compiler.RuntimeLibrary.MultiThreadedDLL);
        }
        #endregion
        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        #region Blobs and unitys
        //[Configure(Blob.FastBuildUnitys)]
        //[ConfigurePriority(ConfigurePriorities.Blobbing)]
        //public virtual void FastBuildUnitys(Configuration conf, CommonTarget target)
        //{
        //    conf.FastBuildBlobbed = true;
        //    conf.FastBuildUnityPath = Path.Combine(Globals.TmpDirectory, @"unity\[project.Name]");
        //    conf.IncludeBlobbedSourceFiles = false;
        //    conf.IsBlobbed = false;
        //}

        //[Configure(Blob.NoBlob)]
        //[ConfigurePriority(ConfigurePriorities.Blobbing)]
        //public virtual void BlobNoBlob(Configuration conf, CommonTarget target)
        //{
        //    conf.FastBuildBlobbed = false;
        //    conf.IsBlobbed = false;

        //    if (conf.IsFastBuild)
        //        conf.ProjectName += "_NoBlob";
        //}
        #endregion
        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        #region Build system
        [ConfigurePriority(ConfigurePriorities.BuildSystem)]
        [Configure(BuildSystem.MSBuild)]
        public virtual void ConfigureMSBuild(Configuration conf, CommonTarget target)
        {
            conf.Options.Add(Options.Vc.Compiler.MultiProcessorCompilation.Enable);
            conf.Defines.Add("NOMINMAX");
        }
        #endregion
        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        #region Compilers and toolchains
        [ConfigurePriority(ConfigurePriorities.Compiler)]
        [Configure(Compiler.MSVC)]
        public virtual void ConfigureMSVC(Configuration conf, CommonTarget target)
        {
        }

        [ConfigurePriority(ConfigurePriorities.Compiler)]
        [Configure(Compiler.ClangCl)]
        public virtual void ConfigureClangCl(Configuration conf, CommonTarget target)
        {
            conf.Options.Add(Options.Vc.General.PlatformToolset.ClangCL);
            conf.Options.Add(Options.Clang.Compiler.ExtraWarnings.Disable);

            conf.AdditionalCompilerOptions.Add(
                "-Wno-c++98-compat",
                "-Wno-microsoft-include",
                "-Wno-ignored-qualifiers",
                "-Wno-unused-parameter",
                "-Wno-comment",
                "-Wno-unused-function",
                "-Wno-missing-braces",
                "-Wno-return-type-c-linkage"
            );
        }
        #endregion
        ////////////////////////////////////////////////////////////////////////
    }

    public abstract class CommonThirdPartyProject : CommonProject
    {
        protected CommonThirdPartyProject() : base()
        { 
            RootPath = Globals.ThirdPartyDirectory;
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.SolutionFolder = "ThirdParty";

            conf.Options.Add(Options.Vc.General.WarningLevel.Level0);
        }

        public override void ConfigureClangCl(Configuration conf, CommonTarget target)
        {
            base.ConfigureClangCl(conf, target);

            conf.AdditionalCompilerOptions.Add(
                "-Wno-everything"
            );
        }
    }

    public abstract class CommonThirdPartyDllProject : CommonThirdPartyProject
    {
        protected CommonThirdPartyDllProject() : base()
        { }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.Output = Configuration.OutputType.Dll;

            if (target.Platform == Platform.win64)
            {
                conf.EventPostBuild.Add(@"copy /Y " + "\"" + conf.TargetPath + "\\" + Name + ".dll\"" + " \"" + Globals.EngineDirectory + "\"");
            }

        }
    }

    public abstract class CommonThirdPartyLibProject : CommonThirdPartyProject
    {
        protected CommonThirdPartyLibProject() : base()
        { }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.Output = Configuration.OutputType.Lib;
        }
    }

    public abstract class CommonVoltProject : CommonProject
    {
        protected CommonVoltProject() : base()
        { }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.Options.Add(Options.Vc.General.TreatWarningsAsErrors.Enable);
            conf.Options.Add(Options.Vc.General.ExternalWarningLevel.Level0);
            conf.Options.Add(Options.Vc.General.TreatAngleIncludeAsExternal.Enable);

            if (this.GetType() != typeof(CoreUtilities))
            {
                conf.AddPublicDependency<CoreUtilities>(target);
            }

            conf.IncludePrivatePaths.Add("Private/");
            conf.IncludePrivatePaths.Add("PCH/");
            conf.IncludePaths.Add("Public/");

            //TEMP
            int moduleIndex = Name.IndexOf("Module");
            if (moduleIndex != -1)
            {
                string modulelessName = Name.Substring(0, moduleIndex);

                conf.IncludePrivatePaths.Add("Public/" + modulelessName);
                conf.IncludePrivatePaths.Add("Private/" + modulelessName);
            }

            conf.IncludePrivatePaths.Add("Public/" + Name);
            conf.IncludePrivatePaths.Add("Private/" + Name);

            conf.Options.Add(new Sharpmake.Options.Vc.Compiler.DisableSpecificWarnings("4005", "4100", "4201", "4251", "4275", "4505"));
            conf.Options.Add(new Sharpmake.Options.Vc.Linker.DisableSpecificWarnings("4006", "4099"));   
        }
    }

    public abstract class CommonVoltExeProject : CommonVoltProject
    {
        protected CommonVoltExeProject() : base()
        { }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.Output = Configuration.OutputType.Exe;

            if (target.Platform == Platform.win64)
            {
                conf.EventPostBuild.Add(@"copy /Y " + "\"" + conf.TargetPath + "\\" + Name + ".exe\"" + " \"" + Globals.EngineDirectory + "\"");
            }
        }
    }

    public abstract class CommonVoltDllProject : CommonVoltProject
    {
        protected CommonVoltDllProject() : base()
        { }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.Output = Configuration.OutputType.Dll;

            if (target.Platform == Platform.win64)
            {
                conf.EventPostBuild.Add(@"copy /Y " + "\"" + conf.TargetPath + "\\" + Name + ".dll\"" + " \"" + Globals.EngineDirectory + "\"");
            }

            string UpperProjectName = Name.ToUpper(); 

            conf.Defines.Add(UpperProjectName + "_DLL_EXPORT");
            conf.ExportDefines.Add(UpperProjectName + "_DLL_IMPORT");
        }
    }

    public abstract class CommonVoltLibProject : CommonVoltProject
    {
        protected CommonVoltLibProject() : base()
        { }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.Output = Configuration.OutputType.Lib;
        }
    }
}
