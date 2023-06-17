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
        public static void InstantiatePrefab(AssetHandle asset, uint spawnPoint)
        {
            InternalCalls.NetScene_InstantiatePrefabAtEntity(asset, spawnPoint);
        }

        public static void DestroyPrefabFromLocal(uint entity)
        {
            InternalCalls.NetScene_DestroyFromLocalId(entity);
        }

        public static void DestroyPrefabFromNet(UInt64 entity)
        {
            InternalCalls.NetScene_DestroyFromNetId(entity);
        }

        public static void Reload()
        {
            InternalCalls.Net_Reload();
        }
        public static void Notify(uint entity, string fieldName)
        {
            InternalCalls.Net_Notify(entity, fieldName);
        }

        public static void NotifyFromNet(UInt64 entity, string fieldName)
        {
            InternalCalls.Net_NotifyFromNetId(entity, fieldName);
        }

        public static UInt64 GetRepId(uint id)
        {
            return InternalCalls.NetActorComponent_GetRepId(id);
        }
        public static void EventFromLocalId(uint id, eNetEvent netEvent, params object[] args)
        {
            byte[] packedArgs = ArgumentPacker.PackArgs(args);
            InternalCalls.NetEvent_TriggerEventFromLocalId(id, netEvent, packedArgs);
        }
        public static void EventFromNetId(UInt64 id, eNetEvent netEvent, params object[] args)
        {
            byte[] packedArgs = ArgumentPacker.PackArgs(args);
            InternalCalls.NetEvent_TriggerEventFromNetId(id, netEvent, packedArgs);
        }
    }

    public static class Net
    {
        static Action ConnectCallback = null;

        public static void SetConnectCallback(Action callback)
        {
            ConnectCallback = callback;
        }

        internal static void OnConnectCallback()
        {
            if (ConnectCallback != null)
            {
                ConnectCallback();
            }
        }

        public static bool IsHost()
        {
            return InternalCalls.Net_IsHost();
        }

        public static void StartClient()
        {
            InternalCalls.Net_StartClient();
        }

        public static void StartServer()
        {
            InternalCalls.Net_StartServer();
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

        public static void InstantiatePlayer()
        {
            InternalCalls.Net_InstantiatePlayer();
        }

        public static ushort GetBoundPort()
        {
            return InternalCalls.Net_GetBoundPort();
        }

        public static void ForceSetPort(ushort port)
        {
            InternalCalls.Net_ForcePortBinding(port);
        }
    }
}