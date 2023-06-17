using System;
using System.Runtime.InteropServices;
using System.Security;

namespace Volt
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Vector2
    {
        public static Vector2 Zero = new Vector2(0, 0);
        public static Vector2 One = new Vector2(1, 1);

        public static Vector2 Right = new Vector2(1, 0);
        public static Vector2 Left = new Vector2(-1, 0);
        public static Vector2 Up = new Vector2(0, 1);
        public static Vector2 Down = new Vector2(0, -1);

        public float x;
        public float y;

        public Vector2(float scalar) => x = y = scalar;

        public Vector2(float x, float y)
        {
            this.x = x;
            this.y = y;
        }

        public Vector2(Vector3 vector)
        {
            x = vector.x;
            y = vector.y;
        }

        [SecuritySafeCritical]
        public unsafe byte[] GetBytes()
        {
            byte[] bytes = new byte[sizeof(Vector2)];
            fixed (byte* ptr = bytes)
            {
                *(Vector2*)ptr = this;
            }

            return bytes;
        }

        public void Clamp(Vector2 min, Vector2 max)
        {
            x = Mathf.Clamp(x, min.x, max.x);
            y = Mathf.Clamp(y, min.y, max.y);
        }

        public float Length() => (float)Math.Sqrt(x * x + y * y);

        public Vector2 Normalized()
        {
            float length = Length();
            return length > 0.0f ? New(v => v / length) : new Vector2(x, y);
        }

        public void Normalize()
        {
            float length = Length();
            if (length == 0.0f)
                return;

            Apply(v => v / length);
        }

        public float Distance(Vector3 other)
        {
            return (float)Math.Sqrt(Math.Pow(other.x - x, 2) +
                                    Math.Pow(other.y - y, 2));
        }

        public static float Distance(Vector3 p1, Vector3 p2)
        {
            return (float)Math.Sqrt(Math.Pow(p2.x - p1.x, 2) +
                                    Math.Pow(p2.y - p1.y, 2));
        }

        //Lerps from p1 to p2
        public static Vector2 Lerp(Vector2 p1, Vector2 p2, float maxDistanceDelta)
        {
            if (maxDistanceDelta < 0.0f)
                return p1;
            if (maxDistanceDelta > 1.0f)
                return p2;

            return p1 + ((p2 - p1) * maxDistanceDelta);
        }

        /// <summary>
        /// Allows you to pass in a delegate that takes in
        /// a double to process the vector per axis.
        /// i.e. (Math.Cos) or a lambda (v => v * 3)
        /// </summary>
        /// <param name="func">Delegate 'double' method to act as a scalar to process X and Y</param>
        public void Apply(Func<double, double> func)
        {
            x = (float)func(x);
            y = (float)func(y);
        }

        /// <summary>
        /// Allows you to pass in a delegate that takes in and returns
        /// a new Vector processed per axis.
        /// i.e. (Math.Cos) or a lambda (v => v * 3)
        /// </summary>
        /// <param name="func">Delegate 'double' method to act as a scalar to process X and Y</param>
        public Vector2 New(Func<double, double> func) => new Vector2((float)func(x), (float)func(y));

        public static Vector2 operator *(Vector2 left, float scalar) => new Vector2(left.x * scalar, left.y * scalar);
        public static Vector2 operator *(float scalar, Vector2 right) => new Vector2(scalar * right.x, scalar * right.y);
        public static Vector2 operator *(Vector2 left, Vector2 right) => new Vector2(left.x * right.x, left.y * right.y);
        public static Vector2 operator /(Vector2 left, Vector2 right) => new Vector2(left.x / right.x, left.y / right.y);
        public static Vector2 operator /(Vector2 left, float scalar) => new Vector2(left.x / scalar, left.y / scalar);
        public static Vector2 operator /(float scalar, Vector2 right) => new Vector2(scalar / right.x, scalar / right.y);
        public static Vector2 operator +(Vector2 left, Vector2 right) => new Vector2(left.x + right.x, left.y + right.y);
        public static Vector2 operator +(Vector2 left, float right) => new Vector2(left.x + right, left.y + right);
        public static Vector2 operator -(Vector2 left, Vector2 right) => new Vector2(left.x - right.x, left.y - right.y);
        public static Vector2 operator -(Vector2 vector) => new Vector2(-vector.x, -vector.y);

        public override string ToString() => "Vector2[" + x + ", " + y + "]";
    }
}
