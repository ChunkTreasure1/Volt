using System;
using System.Runtime.InteropServices;

namespace Volt
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Color
    {
        public static Color Zero = new Color(0, 0, 0, 0);
        public static Color One = new Color(1, 1, 1, 1);

        public float x;
        public float y;
        public float z;
        public float w;

        public Color(float scalar) => x = y = z = w = scalar;

        public Color(float x, float y, float z, float w)
        {
            this.x = x;
            this.y = y;
            this.z = z;
            this.w = w;
        }

        public Color(uint x, uint y, uint z, uint w)
        {
            this.x = x / 255f;
            this.y = y / 255f;
            this.z = z / 255f;
            this.w = w / 255f;
        }

        public Vector4 AsVector4()
        {
            return new Vector4(x, y, z, w);
        }

        public static Color Lerp(Color a, Color b, float t)
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

        /// <summary>
        /// Allows you to pass in a delegate that takes in and returns
        /// a new Vector processed per axis, retaining W.
        /// i.e. (Math.Cos) or a lambda (v => v * 3)
        /// </summary>
        /// <param name="func">Delegate 'double' method to act as a scalar to process X, Y and Z, retaining W</param>
        public Color New(Func<double, double> func) => new Color((float)func(x), (float)func(y), (float)func(z), w);

        public static Color operator +(Color left, Color right) => new Color(left.x + right.x, left.y + right.y, left.z + right.z, left.w + right.w);
        public static Color operator -(Color left, Color right) => new Color(left.x - right.x, left.y - right.y, left.z - right.z, left.w - right.w);
        public static Color operator *(Color left, Color right) => new Color(left.x * right.x, left.y * right.y, left.z * right.z, left.w * right.w);
        public static Color operator *(Color left, float scalar) => new Color(left.x * scalar, left.y * scalar, left.z * scalar, left.w * scalar);
        public static Color operator *(float scalar, Color right) => new Color(scalar * right.x, scalar * right.y, scalar * right.z, scalar * right.w);
        public static Color operator /(Color left, Color right) => new Color(left.x / right.x, left.y / right.y, left.z / right.z, left.w / right.w);
        public static Color operator /(Color left, float scalar) => new Color(left.x / scalar, left.y / scalar, left.z / scalar, left.w / scalar);
    }
}
