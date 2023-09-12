using System;
using Microsoft.VisualStudio.Shell;

namespace VoltVSTools
{
	internal class VoltSolutionEventsListener : SolutionEventsListener
	{
		public VoltSolutionEventsListener(IServiceProvider serviceProvider)
			: base(serviceProvider)
		{
			ThreadHelper.ThrowIfNotOnUIThread();
			Init();
		}

		public string SolutionDirectory
		{
			get
			{
				ThreadHelper.ThrowIfNotOnUIThread();
				Solution.GetSolutionInfo(out string solutionDirectory, out string solutionFile, out string userOptsFile);
				_ = solutionFile;
				_ = userOptsFile;
				return solutionDirectory;
			}
		}
	}
}
