using EnvDTE;
using Mono.Debugging.Soft;
using Mono.Debugging.VisualStudio;

namespace VoltVSTools.Debugging
{
    public enum VoltSessionType
    {
        PlayInEditor = 0,
        AttachVoltDebugger
    }

    internal class VoltStartInfo : StartInfo
    {
        public readonly VoltSessionType SessionType;

        public VoltStartInfo(SoftDebuggerStartArgs args, DebuggingOptions options, Project startupProject, VoltSessionType sessionType)
            : base(args, options, startupProject)
        {
            SessionType = sessionType;
        }
    }
}
