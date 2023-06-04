using System;

namespace Volt
{
    public struct AssetHandle
    {
        private System.UInt64 handle;

        private AssetHandle(System.UInt64 aHandle)
        {
            this.handle = aHandle;
        }

        public static implicit operator AssetHandle(System.UInt64 aHandle)
        {
            return new AssetHandle(aHandle);
        }

        public static implicit operator System.UInt64(AssetHandle aHandle)
        {
            return aHandle.handle;
        }

        public override string ToString()
        {
            return handle.ToString();
        }
    }

    public class Asset
    {
        public AssetHandle handle;
        
        public Asset() { }
        public Asset(AssetHandle handle)
        {
            this.handle = handle;
        }

        public override string ToString()
        {
            return handle.ToString();
        }

        public bool IsValid()
        {
            return InternalCalls.Asset_IsValid(handle);
        }
    }
}
