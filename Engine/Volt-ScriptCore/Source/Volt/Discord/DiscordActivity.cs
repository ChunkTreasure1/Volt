using System;
using System.Runtime.InteropServices;

// TBC :)

namespace Volt
{
    public enum ActivityType
    {
        Playing,
        Streaming,
        Listening,
        Watching,
    }

    public enum ActivityPartyPrivacy
    {
        Private = 0,
        Public = 1,
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public partial struct ActivityTimestamps
    {
        public Int64 Start;

        public Int64 End;
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public partial struct ActivityAssets
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string LargeImage;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string LargeText;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string SmallImage;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string SmallText;
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public partial struct ActivityParty
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string Id;

        public PartySize Size;

        public ActivityPartyPrivacy Privacy;
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public partial struct ActivitySecrets
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string Match;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string Join;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string Spectate;
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public partial struct PartySize
    {
        public Int32 CurrentSize;
        public Int32 MaxSize;
    }

    public partial struct Activity
    {
        public ActivityType Type;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string Name;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string State;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string Details;

        public ActivityTimestamps Timestamps;
        public ActivityAssets Assets;
        public ActivityParty Party;
        public ActivitySecrets Secrets;

        public bool Instance;
    }
}
