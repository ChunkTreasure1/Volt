using System;
using System.IO;

using Sharpmake;

namespace Volt
{
    [Sharpmake.Generate]
    public class DiscordSDK : CommonThirdPartyLibProject
    {
        public DiscordSDK() : base()
        {
            Name = "DiscordSDK";

            SourceRootPath = @"[project.RootPath]\[project.Name]";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.IncludePaths.Add(@"include\discord");

            conf.LibraryFiles.Add("discord_game_sdk.dll.lib");
            conf.LibraryPaths.Add("lib/");
        }
    }
}
