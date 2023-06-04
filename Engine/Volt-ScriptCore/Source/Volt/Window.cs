using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Volt
{
    public static class Window
    {
        public static float GetWidth()
        {
            return InternalCalls.Window_GetWidth();
        }

        public static float GetHeight()
        {
            return InternalCalls.Window_GetHeight();
        }
    }
}
