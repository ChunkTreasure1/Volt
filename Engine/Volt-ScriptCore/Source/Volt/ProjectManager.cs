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
        public static string GetRootDirectory()
        {
            return InternalCalls.Project_GetDirectory();
        }

        public static string GetProjectName()
        {
            return InternalCalls.Project_GetProjectName();
        }

        public static string GetCompanyName()
        {
            return InternalCalls.Project_GetCompanyName();
        }
    }
}
