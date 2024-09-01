using System;
using System.IO;

using Sharpmake;

namespace Volt
{
    [Sharpmake.Generate]
    public class DirectXTex : CommonThirdPartyLibProject
    {
        public DirectXTex() : base()
        {
            Name = "DirectXTex";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.IncludePaths.Add("src/DirectXTex");
        }
    }
}
