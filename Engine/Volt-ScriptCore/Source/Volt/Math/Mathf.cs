using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Volt
{
    public static class Mathf
    {
        public static float Sqrt(float f)
        {
            return (float)Math.Sqrt(f);
        }

        public static float Radians(float degrees)
        {
            return (degrees * 0.01745329251994329576923690768489f);
        }

        public static float Degrees(float radians)
        {
            return (radians * 57.295779513082320876798154814105f);
        }

        public static Vector3 Radians(Vector3 degrees)
        {
            return new Vector3(Radians(degrees.x), Radians(degrees.y), Radians(degrees.z));
        }

        public static Vector3 Degrees(Vector3 radians)
        {
            return new Vector3(Degrees(radians.x), Degrees(radians.y), Degrees(radians.z));
        }

        public static float Acos(float f)
        {
            return (float)Math.Acos(f);
        }

        public static float Cos(float v)
        {
            return (float)Math.Cos((double)v);
        }

        public static float Sin(float v)
        {
            return (float)Math.Sin((double)v);
        }

        public static Vector3 Cos(Vector3 v)
        {
            return new Vector3(Cos(v.x), Cos(v.y), Cos(v.z));
        }
        public static Vector3 Sin(Vector3 v)
        {
            return new Vector3(Sin(v.x), Sin(v.y), Sin(v.z));
        }

        public static float Clamp(float v, float min, float max)
        {
            return (float)Math.Min(Math.Max(v, min), max);
        }
    }
}
