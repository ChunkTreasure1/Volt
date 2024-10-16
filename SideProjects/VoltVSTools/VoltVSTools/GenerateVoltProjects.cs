﻿using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using System;
using System.ComponentModel.Design;
using System.Diagnostics;
using System.Net;
using System.Reflection;
using Task = System.Threading.Tasks.Task;
using Microsoft.VisualStudio;
using EnvDTE;
using System.Threading.Tasks;
using Microsoft.VisualStudio.OLE.Interop;
using System.IO;
using static System.Net.Mime.MediaTypeNames;

namespace VoltVSTools
{
    /// <summary>
    /// Command handler
    /// </summary>
    internal sealed class GenerateVoltProjects
    {
        /// <summary>
        /// Command ID.
        /// </summary>
        public const int CommandId = 256;

        /// <summary>
        /// Command menu group (command set GUID).
        /// </summary>
        public static readonly Guid CommandSet = new Guid("BED7CE54-DB82-4DF1-8807-3F119077C4D0");

        /// <summary>
        /// VS Package that provides this command, not null.
        /// </summary>
        private readonly AsyncPackage package;

        private IVsSolutionBuildManager m_SolutionBuildManager;

        private MenuCommand m_MenuItem;

        /// <summary>
        /// Initializes a new instance of the <see cref="GenerateVoltProjects"/> class.
        /// Adds our command handlers for menu (commands must exist in the command table file)
        /// </summary>
        /// <param name="package">Owner package, not null.</param>
        /// <param name="commandService">Command service to add command to, not null.</param>
        private GenerateVoltProjects(AsyncPackage package, OleMenuCommandService commandService)
        {
            this.package = package ?? throw new ArgumentNullException(nameof(package));
            commandService = commandService ?? throw new ArgumentNullException(nameof(commandService));

            var menuCommandID = new CommandID(CommandSet, CommandId);
            m_MenuItem = new MenuCommand(this.Execute, menuCommandID);
            commandService.AddCommand(m_MenuItem);
        }

        /// <summary>
        /// Gets the instance of the command.
        /// </summary>
        public static GenerateVoltProjects Instance
        {
            get;
            private set;
        }

        /// <summary>
        /// Gets the service provider from the owner package.
        /// </summary>
        private Microsoft.VisualStudio.Shell.IAsyncServiceProvider ServiceProvider
        {
            get
            {
                return this.package;
            }
        }

        /// <summary>
        /// Initializes the singleton instance of the command.
        /// </summary>
        /// <param name="package">Owner package, not null.</param>
        public static async Task InitializeAsync(AsyncPackage package)
        {
            // Switch to the main thread - the call to AddCommand in AttachVoltnutCommand's constructor requires
            // the UI thread.
            await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync(package.DisposalToken);

            OleMenuCommandService commandService = await package.GetServiceAsync(typeof(IMenuCommandService)) as OleMenuCommandService;
            Instance = new GenerateVoltProjects(package, commandService);
            Instance.m_SolutionBuildManager = await package.GetServiceAsync(typeof(IVsSolutionBuildManager)) as IVsSolutionBuildManager;
        }

        /// <summary>
        /// This function is the callback used to execute the command when the menu item is clicked.
        /// See the constructor to see how the menu item is associated with this function using
        /// OleMenuCommandService service and MenuCommand class.
        /// </summary>
        /// <param name="sender">Event sender.</param>
        /// <param name="e">Event args.</param>
        private async void Execute(object sender, EventArgs e)
        {
            await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync(); // Ensure UI thread for IVsOutputWindow access

            string solutionDir = VoltToolsPackage.Instance.SolutionEventsListener?.SolutionDirectory;

            if (solutionDir != null)
            {
                string generateProjectsFilePath = Path.Combine(solutionDir, "GenerateProjects.bat");

                System.Diagnostics.Process proc = new System.Diagnostics.Process();
                proc.StartInfo.FileName = generateProjectsFilePath;
                proc.StartInfo.CreateNoWindow = true;
                proc.StartInfo.RedirectStandardOutput = true;
                proc.StartInfo.UseShellExecute = false;

                //// Get the Output window pane
                //IVsOutputWindow outputWindow = await ServiceProvider.GetServiceAsync<SVsOutputWindow, IVsOutputWindow>();
                //Guid generalPaneGuid = VSConstants.SID_SVsGeneralOutputWindowPane; // Or create your own pane
                //IVsOutputWindowPane generalPane;
                //outputWindow.GetPane(ref generalPaneGuid, out generalPane);

                // Get the IVsOutputWindow service
                var outputWindow = (IVsOutputWindow)Package.GetGlobalService(typeof(SVsOutputWindow));

                if (outputWindow == null)
                {
                    // Handle error
                    return;
                }

                // Define the output pane GUID and name
                var paneGuid = VSConstants.GUID_OutWindowGeneralPane;
                string paneName = "Generate Volt Projects Output";

                // Create or get the output pane
                IVsOutputWindowPane outputPane;
                int result = outputWindow.GetPane(ref paneGuid, out outputPane);
                if (result != VSConstants.S_OK || outputPane == null)
                {
                    // Create a new pane if it doesn't exist
                    outputWindow.CreatePane(ref paneGuid, paneName, 1, 1);
                    outputWindow.GetPane(ref paneGuid, out outputPane);
                }

                outputPane.Clear();
                outputPane.Activate();

                proc.OutputDataReceived += async (sendingProcess, outputLine) =>
                {
                    await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();

                    if (!string.IsNullOrEmpty(outputLine.Data))
                    {
                        outputPane?.OutputStringThreadSafe(outputLine.Data + Environment.NewLine);
                    }
                };

                proc.Start();
                proc.BeginOutputReadLine();
            }
        }
    }
}
