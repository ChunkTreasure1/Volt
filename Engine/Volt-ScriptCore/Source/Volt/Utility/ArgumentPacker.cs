using System;
using System.IO;

namespace Volt
{
    // Packing:
    // Args count : 1 bytes
    // Arg size: 1 bytes
    // Arg: Arg size
    // ...

    public static class ArgumentPacker
    {
        private static byte[] GetBytes(object obj)
        {
            if (obj.GetType() == typeof(float)) { return BitConverter.GetBytes((float)obj); }
            else if (obj.GetType() == typeof(double)) { return BitConverter.GetBytes((double)obj); }
            else if (obj.GetType() == typeof(int)) { return BitConverter.GetBytes((int)obj); }
            else if (obj.GetType() == typeof(long)) { return BitConverter.GetBytes((long)obj); }
            else if (obj.GetType() == typeof(ulong)) { return BitConverter.GetBytes((ulong)obj); }
            else if (obj.GetType() == typeof(bool)) { return BitConverter.GetBytes((bool)obj); }
            else if (obj.GetType() == typeof(uint)) { return BitConverter.GetBytes((uint)obj); }
            else if (obj.GetType() == typeof(byte)) { return BitConverter.GetBytes((byte)obj); }

            return null;
        }

        public static byte[] PackArgs(params object[] args)
        {
            MemoryStream stream = new MemoryStream();
            BinaryWriter bw = new BinaryWriter(stream);
            bw.Write((byte)args.Length);

            foreach (object arg in args)
            {
                var bytes = GetBytes(arg);

                if (bytes == null)
                {
                    bw.Write((byte)0);
                }
                else
                {
                    bw.Write((byte)bytes.Length);
                    bw.Write(bytes);
                }
            }

            return stream.ToArray();
        }
    }
}
