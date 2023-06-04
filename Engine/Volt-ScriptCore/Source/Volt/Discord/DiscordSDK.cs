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
    }
}
