using System;
using System.Runtime.InteropServices;
using System.Threading;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Mono.Debugging.Client;
using Task = System.Threading.Tasks.Task;

namespace VoltVSTools
{
	/// <summary>
	/// This is the class that implements the package exposed by this assembly.
	/// </summary>
	/// <remarks>
	/// <para>
	/// The minimum requirement for a class to be considered a valid package for Visual Studio
	/// is to implement the IVsPackage interface and register itself with the shell.
	/// This package uses the helper classes defined inside the Managed Package Framework (MPF)
	/// to do it: it derives from the Package class that provides the implementation of the
	/// IVsPackage interface and uses the registration attributes defined in the framework to
	/// register itself and its components with the shell. These attributes tell the pkgdef creation
	/// utility what data to put into .pkgdef file.
	/// </para>
	/// <para>
	/// To get loaded into VS, the package must be referred by &lt;Asset Type="Microsoft.VisualStudio.VsPackage" ...&gt; in .vsixmanifest file.
	/// </para>
	/// </remarks>
	[PackageRegistration(UseManagedResourcesOnly = true, AllowsBackgroundLoading = true)]
	[Guid(PackageGuidString)]
	[ProvideMenuResource("Menus.ctmenu", 1)]
	[ProvideUIContextRule(UIContextHasCSProjectGuid, name: "Has CSharp Project",
		expression: "HasCSProject",
		termNames: new[] { "HasCSProject" },
		termValues: new[] { "SolutionHasProjectCapability:CSharp" })]
	public sealed class VoltToolsPackage : AsyncPackage
	{
		/// <summary>
		/// GodotPackage GUID string.
		/// </summary>
		public const string PackageGuidString = "010FEDBE-99D0-45B9-AA1A-FF11E4C5E9D0";
		private const string UIContextHasCSProjectGuid = "B95AF128-812D-47D0-B875-6398CF6DD004";

		#region Package Members

		public static VoltToolsPackage Instance { get; private set; }

		public VoltToolsPackage()
		{
			Instance = this;
		}

		internal VoltSolutionEventsListener SolutionEventsListener { get; private set; }

		/// <summary>
		/// Initialization of the package; this method is called right after the package is sited, so this is the place
		/// where you can put all the initialization code that rely on services provided by VisualStudio.
		/// </summary>
		/// <param name="cancellationToken">A cancellation token to monitor for initialization cancellation, which can occur when VS is shutting down.</param>
		/// <param name="progress">A provider for progress updates.</param>
		/// <returns>A task representing the async work of package initialization, or an already completed task if there is none. Do not return null from this method.</returns>
		protected override async Task InitializeAsync(CancellationToken cancellationToken, IProgress<ServiceProgressData> progress)
		{
			// When initialized asynchronously, the current thread may be a background thread at this point.
			// Do any initialization that requires the UI thread after switching to the UI thread.
			await this.JoinableTaskFactory.SwitchToMainThreadAsync(cancellationToken);
			await AttachVoltCommand.InitializeAsync(this);

			SolutionEventsListener = new VoltSolutionEventsListener(this);
		}

		public async Task ShowErrorMessageBoxAsync(string title, string message)
		{
			await JoinableTaskFactory.SwitchToMainThreadAsync();

			var uiShell = (IVsUIShell)await GetServiceAsync(typeof(SVsUIShell));

			if (uiShell == null)
				throw new ServiceUnavailableException(typeof(SVsUIShell));

			var clsID = Guid.Empty;
			Microsoft.VisualStudio.ErrorHandler.ThrowOnFailure(uiShell.ShowMessageBox(
				0,
				ref clsID,
				title,
				message,
				string.Empty,
				0,
				OLEMSGBUTTON.OLEMSGBUTTON_OK,
				OLEMSGDEFBUTTON.OLEMSGDEFBUTTON_FIRST,
				OLEMSGICON.OLEMSGICON_CRITICAL,
				0,
				pnResult: out _
			));
		}

		#endregion
	}
}