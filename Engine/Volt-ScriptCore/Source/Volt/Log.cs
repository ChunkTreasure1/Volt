using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Volt
{
    public enum LogLevel : uint
    {
        Trace,
        Info,
        Warning,
        Error,
        Critical
    }

    public static class Log
    {
        public static void Trace(string text)
        {
            InternalCalls.Log_String(ref text, LogLevel.Trace);
        }

        public static void Info(string text)
        {
            InternalCalls.Log_String(ref text, LogLevel.Info);
        }

        public static void Warning(string text)
        {
            InternalCalls.Log_String(ref text, LogLevel.Warning);
        }

        public static void Error(string text)
        {
            InternalCalls.Log_String(ref text, LogLevel.Error);
        }

        public static void Critical(string text)
        {
            InternalCalls.Log_String(ref text, LogLevel.Critical);
        }
    }
}
