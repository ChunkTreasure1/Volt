using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Volt
{
    public static class AssetManager
    {
        static Dictionary<AssetHandle, Asset> myAssets = new Dictionary<AssetHandle, Asset>();

        public static T GetAsset<T>(AssetHandle handle) where T : Asset, new()
        {
            if (!InternalCalls.AssetManager_HasAsset(handle))
            {
                return null;
            }

            if (myAssets.ContainsKey(handle))
            {
                return myAssets[handle] as T;
            }

            T asset = new T();
            asset.handle = handle;

            myAssets.Add(handle, asset);
            return asset;
        }

        public static T GetAsset<T>(string path) where T : Asset, new()
        {
            ulong handle = InternalCalls.AssetManager_GetAssetHandleFromPath(path);

            if (handle == 0) { return null; }

            if (!InternalCalls.AssetManager_HasAsset(handle))
            {
                return null;
            }

            if (myAssets.ContainsKey(handle))
            {
                return myAssets[handle] as T;
            }

            T asset = new T();
            asset.handle = handle;

            myAssets.Add(handle, asset);
            return asset;
        }

        public static AssetHandle GetAssetHandleFromPath(string path)
        {
            return InternalCalls.AssetManager_GetAssetHandleFromPath(path);
        }
    }
}
