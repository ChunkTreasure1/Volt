using System;

namespace Volt
{
    static public class SteamAPI
    {
        static Action<string> JoinRequestCallback = null;

        public static void StartLobby(string address)
        {
            InternalCalls.SteamAPI_StartLobby(address);
        }

        public static void SetOnJoinRequest(Action<string> callback)
        {
            JoinRequestCallback = callback;
        }

        internal static void Clean()
        {
            JoinRequestCallback = null;
        }

        internal static void OnJoinRequest(string address)
        {
            if (JoinRequestCallback != null)
            {
                JoinRequestCallback(address);
            }
        }

        public static void SetStat(string name, int value)
        {
            InternalCalls.SteamAPI_SetStatInt(name, value);
        }

        public static void SetStat(string name, float value)
        {
            InternalCalls.SteamAPI_SetStatFloat(name, value);
        }

        public static float GetStatFloat(string name)
        {
            return InternalCalls.SteamAPI_GetStatFloat(name);
        }

        public static int GetStatInt(string name)
        {
            return InternalCalls.SteamAPI_GetStatInt(name);
        }

        public static bool IsStatsValid()
        {
            return InternalCalls.SteamAPI_IsStatsValid();
        }

        public static void StoreStats()
        {
            InternalCalls.SteamAPI_StoreStats();
        }

        public static void RequestStats()
        {
            InternalCalls.SteamAPI_RequestStats();
        }
    }
}
