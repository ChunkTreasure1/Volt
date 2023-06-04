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
#if DIST
        public static void Trace(string text, string category = "") { }
        public static void Info(string text, string category = "") { }
        public static void Warning(string text, string category = "") { }
        public static void Error(string text, string category = "") { }
        public static void Critical(string text, string category = "") { }
#else
        public static void Trace(string text, string category = "")
        {
            if (category.Length > 0)
            {
                text = "$[" + category + "]" + text;
            }
            InternalCalls.Log_String(text, LogLevel.Trace);
        }

        public static void Info(string text, string category = "")
        {
            if (category.Length > 0)
            {
                text = "$[" + category + "]" + text;
            }
            InternalCalls.Log_String(text, LogLevel.Info);
        }

        public static void Warning(string text, string category = "")
        {
            if (category.Length > 0)
            {
                text = "$[" + category + "]" + text;
            }
            InternalCalls.Log_String(text, LogLevel.Warning);
        }

        public static void Error(string text, string category = "")
        {
            if (category.Length > 0)
            {
                text = "$[" + category + "]" + text;
            }
            InternalCalls.Log_String(text, LogLevel.Error);
        }

        public static void Critical(string text, string category = "")
        {
            if (category.Length > 0)
            {
                text = "$[" + category + "]" + text;
            }
            InternalCalls.Log_String(text, LogLevel.Critical);
        }
#endif
    }
}
