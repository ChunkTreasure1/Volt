using System;

namespace Volt
{
    public class Animation : Asset
    {
        public Animation() { }
        public Animation(AssetHandle handle) : base(handle) { }

        public float Duration
        {
            get
            {
                return InternalCalls.Animation_GetDuration(handle);
            }
        }
    }
}
