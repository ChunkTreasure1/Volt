using System;
using System.Runtime.InteropServices;
using System.Security.AccessControl;
using System.Security.Principal;

namespace Volt
{
    [AttributeUsage(AttributeTargets.Field)]
    public class RepNotify : Attribute
    {
        public RepNotify(string data = "") { function = data; }
        public string function = "";
    }

    [AttributeUsage(AttributeTargets.Field)]
    public class RepContinuous : Attribute
    {
        public RepContinuous(string data = "") { function = data; }
        public string function = "";
    }

    [AttributeUsage(AttributeTargets.Field)]
    public class RepUpdate : Attribute
    {
        public RepUpdate(string data = "") { function = data; }
        public string function = "";
    }


    [AttributeUsage(AttributeTargets.Field)]
    public class HideInEditorAttribute : Attribute { }


    [AttributeUsage(AttributeTargets.Class)]
    public class EngineScript : Attribute { }


}
