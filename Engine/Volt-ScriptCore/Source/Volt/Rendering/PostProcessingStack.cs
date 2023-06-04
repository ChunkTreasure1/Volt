using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Volt
{
    public static class PostProcessingStack
    {
        public static void PushEffect(PostProcessingMaterial material)
        {
            InternalCalls.PostProcessingStack_PushEffect(material.handle);
        }

        public static void PopEffect() 
        {
            InternalCalls.PostProcessingStack_PopEffect();
        }
    }
}
