using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Volt
{
    // Needs to match c++
    public enum eNetEvent : byte
    {
        NIL,
        Hit,
        Death,
        OnCreation,
        OnDestruction,
        Animation,
        Interact
    }

    public static class NetScene
    {
        //void NotifyLocal(ulong scriptId, string variable)
        //{

        //}

        public static void InstantiatePrefab(AssetHandle asset, uint spawnPoint)
        {
            InternalCalls.NetScene_InstantiatePrefabAtEntity(asset, spawnPoint);
        }

        public static void DestroyPrefabFromLocal(uint entity)
        {
            InternalCalls.NetScene_DestroyFromLocalId(entity);
        }

        public static void DestroyPrefabFromNet(UInt16 entity)
        {
            InternalCalls.NetScene_DestroyFromNetId(entity);

        }
    }

    public static class Net
    {
        public static void Notify(uint entity, string fieldName)
        {
            InternalCalls.Net_Notify(entity, fieldName);
        }

        public static void NotifyFromNet(UInt16 entity, string fieldName)
        {
            InternalCalls.Net_NotifyFromNetId(entity, fieldName);
        }

        public static void SceneLoad()
        {
            InternalCalls.Net_SceneLoad();
        }

        public static void StartClient(ushort port)
        {
            InternalCalls.Net_StartClient(port);
        }

        public static void StartServer(ushort port)
        {
            InternalCalls.Net_StartServer(port);
        }

        public static void StartSinglePlayer()
        {
            InternalCalls.Net_StartSinglePlayer();
        }

        public static void SetHandleTick(bool value)
        {
            InternalCalls.Net_SetHandleTick(value);
        }

        public static void Connect(string ip = "", ushort port = 0)
        {
            InternalCalls.Net_Connect(ip, port);
        }

        public static void Disconnect()
        {
            InternalCalls.Net_Disconnect();
        }
    }
}