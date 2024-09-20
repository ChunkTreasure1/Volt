using System;
using System.IO;

namespace VoltSharpmake
{
	[Sharpmake.Generate]
	public class Circuit : CommonVoltDllProject
	{
		public Circuit()
		{
			Name = "Circuit";
		}

		public override void ConfigureAll(Configuration conf, CommonTarget target)
		{
			base.ConfigureAll(conf, target);

			conf.SolutionFolder = "Engine";

			conf.PrecompHeader = "circuitpch.h";
			conf.PrecompSource = "circuitpch.cpp";


			conf.AddPublicDependency<RHIModule>(target);
			conf.AddPublicDependency<RenderCoreModule>(target);
			conf.AddPublicDependency<WindowModule>(target);
			conf.AddPublicDependency<LogModule>(target);
			conf.AddPublicDependency<EventSystemModule>(target);
			conf.AddPublicDependency<InputModule>(target);
			conf.AddPublicDependency<VoltAssets>(target);

			// #TODO_Ivar: These should not be here
			conf.AddPublicDependency<msdfgen>(target);
			conf.AddPublicDependency<msdf_atlas_gen>(target);
		}
	}
}
