using System;
using System.IO;

namespace Volt
{
    [Sharpmake.Generate]
    public class esfw : CommonThirdPartyLibProject
    {
        public esfw()
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "esfw";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.SourceFilesBuildExclude.Add("src/efsw/WatcherKqueue.cpp");
            conf.SourceFilesBuildExclude.Add("src/efsw/WatcherFSEvents.cpp");
            conf.SourceFilesBuildExclude.Add("src/efsw/WatcherInotify.cpp");
            conf.SourceFilesBuildExclude.Add("src/efsw/FileWatcherKqueue.cpp");
            conf.SourceFilesBuildExclude.Add("src/efsw/FileWatcherInotify.cpp");
            conf.SourceFilesBuildExclude.Add("src/efsw/FileWatcherFSEvents.cpp");

            conf.IncludePaths.Add("src");
            conf.IncludePaths.Add("include");
        }

        public override void ConfigureWin64(Configuration conf, CommonTarget target)
        {
            base.ConfigureWin64(conf, target);

            conf.SourceFilesBuildExclude.Add("src/efsw/platform/posix/FileSystemImpl.cpp");
            conf.SourceFilesBuildExclude.Add("src/efsw/platform/posix/MutexImpl.cpp");
            conf.SourceFilesBuildExclude.Add("src/efsw/platform/posix/SystemImpl.cpp");
            conf.SourceFilesBuildExclude.Add("src/efsw/platform/posix/ThreadImpl.cpp");
        }

        public override void ConfigureLinux(Configuration conf, CommonTarget target)
        {
            base.ConfigureLinux(conf, target);

            conf.SourceFilesBuildExclude.Add("src/efsw/platform/win/FileSystemImpl.cpp");
            conf.SourceFilesBuildExclude.Add("src/efsw/platform/win/MutexImpl.cpp");
            conf.SourceFilesBuildExclude.Add("src/efsw/platform/win/SystemImpl.cpp");
            conf.SourceFilesBuildExclude.Add("src/efsw/platform/win/ThreadImpl.cpp");
        }
    }
}
