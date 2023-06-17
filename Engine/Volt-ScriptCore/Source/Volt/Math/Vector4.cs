using System;
using System.Runtime.InteropServices;
using System.Security;

namespace Volt
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Vector4
    {
        public static Vector4 Zero = new Vector4(0, 0, 0, 0);
        public static Vector4 One = new Vector4(1, 1, 1, 1);

        public float x;
        public float y;
        public float z;
        public float w;

        public Vector4(float scalar) => x = y = z = w = scalar;

        public Vector4(float x, float y, float z, float w)
        {
            this.x = x;
            this.y = y;
            this.z = z;
            this.w = w;
        }

        public static Vector4 Lerp(Vector4 a, Vector4 b, float t)
        {
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;
            return (1.0f - t) * a + t * b;
        }

        /// <summary>
        /// Allows you to pass in a delegate that takes in
        /// a double to process the vector per axis, retaining W.
        /// i.e. (Math.Cos) or a lambda (v => v * 3)
        /// </summary>
        /// <param name="func">Delegate 'double' method to act as a scalar to process X, Y and Z, retaining W</param>
        public void Apply(Func<double, double> func)
        {
            x = (float)func(x);
            y = (float)func(y);
            z = (float)func(z);
        }

        [SecuritySafeCritical]
        public unsafe byte[] GetBytes()
        {
            byte[] bytes = new byte[sizeof(Vector4)];
            fixed (byte* ptr = bytes)
            {
                *(Vector4*)ptr = this;
            }

            return bytes;
        }

        /// <summary>
        /// Allows you to pass in a delegate that takes in and returns
        /// a new Vector processed per axis, retaining W.
        /// i.e. (Math.Cos) or a lambda (v => v * 3)
        /// </summary>
        /// <param name="func">Delegate 'double' method to act as a scalar to process X, Y and Z, retaining W</param>
        public Vector4 New(Func<double, double> func) => new Vector4((float)func(x), (float)func(y), (float)func(z), w);

        public static Vector4 operator +(Vector4 left, Vector4 right) => new Vector4(left.x + right.x, left.y + right.y, left.z + right.z, left.w + right.w);
        public static Vector4 operator -(Vector4 left, Vector4 right) => new Vector4(left.x - right.x, left.y - right.y, left.z - right.z, left.w - right.w);
        public static Vector4 operator *(Vector4 left, Vector4 right) => new Vector4(left.x * right.x, left.y * right.y, left.z * right.z, left.w * right.w);
        public static Vector4 operator *(Vector4 left, float scalar) => new Vector4(left.x * scalar, left.y * scalar, left.z * scalar, left.w * scalar);
        public static Vector4 operator *(float scalar, Vector4 right) => new Vector4(scalar * right.x, scalar * right.y, scalar * right.z, scalar * right.w);
        public static Vector4 operator /(Vector4 left, Vector4 right) => new Vector4(left.x / right.x, left.y / right.y, left.z / right.z, left.w / right.w);
        public static Vector4 operator /(Vector4 left, float scalar) => new Vector4(left.x / scalar, left.y / scalar, left.z / scalar, left.w / scalar);
    }
}
