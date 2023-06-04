using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Volt
{
    public static class NetEvents
    {
        public static UInt16 GetRepId(uint id)
        {
            return InternalCalls.NetActorComponent_GetRepId(id);
        }
        public static void EventFromLocalId(uint id, eNetEvent netEvent, params object[] args)
        {
            byte[] packedArgs = ArgumentPacker.PackArgs(args);
            InternalCalls.NetEvent_TriggerEventFromLocalId(id, netEvent, packedArgs);
        }
        public static void EventFromNetId(UInt16 id, eNetEvent netEvent, params object[] args)
        {
            byte[] packedArgs = ArgumentPacker.PackArgs(args);
            InternalCalls.NetEvent_TriggerEventFromNetId(id, netEvent, packedArgs);
        }
    }
}
