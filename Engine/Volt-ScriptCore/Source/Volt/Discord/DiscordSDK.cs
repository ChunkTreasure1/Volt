using System;

namespace Volt
{
    public class Discord
    {
        public static bool Init(long appId)
        {
            InternalCalls.DiscordSDK_Init(appId);
            return true;
        }

        public static void SetDetails(string value)
        {

        }

        public static void SetState(string state)
        {

        }
    }
}
