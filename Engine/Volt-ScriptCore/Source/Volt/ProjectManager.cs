using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Volt;

namespace Volt
{
    public static class ProjectManager
    {
        public static string GetDirectory()
        {
            return InternalCalls.Project_GetDirectory();
        }
    }
}
