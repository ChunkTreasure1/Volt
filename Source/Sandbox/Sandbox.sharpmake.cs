using Sharpmake;

namespace Volt
{
    [Sharpmake.Generate]
    public class Sandbox : CommonVoltExeProject
    {
        public Sandbox()
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "Sandbox";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.PrecompHeader = "sbpch.h";
            conf.PrecompSource = "sbpch.cpp";

            conf.AddPrivateDependency<Volt>(target);
        }
    }
}
