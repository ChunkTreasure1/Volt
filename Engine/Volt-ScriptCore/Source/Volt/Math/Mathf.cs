using System;
using System.Runtime.Remoting.Messaging;

namespace Volt
{
    public static class Mathf
    {

        public const float Epsilon = 0.00001f;
        public const float PI = (float)Math.PI;
        public const float TwoPI = (float)(Math.PI * 2.0);

        public const float Deg2Rad = PI / 180.0f;
        public const float Rad2Deg = 180.0f / PI;

        public static float Degrees(float value)
        {
            return value * Rad2Deg;
        }

        public static float Radians(float value)
        {
            return value * Deg2Rad;
        }

        public static Vector2 Degrees(Vector2 value)
        {
            return value * Rad2Deg;
        }

        public static Vector2 Radians(Vector2 value)
        {
            return value * Deg2Rad;
        }

        public static Vector3 Degrees(Vector3 value)
        {
            return value * Rad2Deg;
        }

        public static Vector3 Radians(Vector3 value)
        {
            return value * Deg2Rad;
        }

        public static Vector4 Degrees(Vector4 value)
        {
            return value * Rad2Deg;
        }

        public static Vector4 Radians(Vector4 value)
        {
            return value * Deg2Rad;
        }

        public static float Sin(float value) => (float)Math.Sin(value);
        public static float Cos(float value) => (float)Math.Cos(value);
        public static float Acos(float value) => (float)Math.Acos(value);

        public static float Clamp(float value, float min, float max)
        {
            if (value < min)
                return min;
            return value > max ? max : value;
        }

        public static float Asin(float x) => (float)Math.Asin(x);
        public static float Atan(float x) => (float)Math.Atan(x);
        public static float Atan2(float y, float x) => (float)Math.Atan2(y, x);

        public static float Min(float v0, float v1) => v0 < v1 ? v0 : v1;
        public static float Max(float v0, float v1) => v0 > v1 ? v0 : v1;

        public static float Sqrt(float value) => (float)Math.Sqrt(value);
        public static float InverseSqrt(float value) => 1f / (float)Math.Sqrt(value);

        public static float Abs(float value) => Math.Abs(value);
        public static int Abs(int value) => Math.Abs(value);

        public static float Pow(float value, float power) => (float)Math.Pow(value, power);




        public static Vector3 Abs(Vector3 value)
        {
            return new Vector3(Math.Abs(value.x), Math.Abs(value.y), Math.Abs(value.z));
        }

        public static float Lerp(float p1, float p2, float t) => Interpolate.Linear(p1, p2, t);
        //lerp angle
        public static float LerpAngle(float p1, float p2, float t)
        {
            float num = Repeat(p2 - p1, 360.0f);
            if (num > 180.0f)
            {
                num -= 360.0f;
            }
            return p1 + num * Clamp(t, 0.0f, 1.0f);
        }

        public static float Repeat(float t, float length) => t - Floor(t / length) * length;


        public static float BounceOut(float p1, float p2, float t, float duration) => Interpolate.BounceOut(t, p1, p2, duration);
        public static float BounceIn(float p1, float p2, float t, float duration)
        {
            return p2 - BounceOut(p1, p2, duration - t, duration);
        }

        public static float BounceLerp(float a, float b, float t)
        {
            const float nl = 7.5625f;
            const float dl = 2.75f;

            if (t < 1.0f / dl)
            {
                return Lerp(a, b, nl * t * t);
            }

            if (t < 2.0f / dl)
            {
                t -= 1.5f / dl;
                return Lerp(a, b, nl * t * t + 0.75f);
            }

            if (t < 2.5f / dl)
            {
                t -= 2.25f / dl;
                return Lerp(a, b, nl * t * t + 0.9375f);
            }

            t -= 2.625f / dl;
            return Lerp(a, b, nl * t * t + 0.984375f);
        }

        public static Vector3 Lerp(Vector3 p1, Vector3 p2, float t) => Interpolate.Linear(p1, p2, t);

        public static float Floor(float value) => (float)Math.Floor(value);
        public static float Ceiling(float value) => (float)Math.Ceiling(value);

        // not the same as a%b
        public static float Modulo(float a, float b) => a - b * (float)Math.Floor(a / b);
        public static float Distance(float p1, float p2) => Abs(p1 - p2);

        public static float Remap(float value, float fromMin, float fromMax, float toMin, float toMax)
        {
            float fromAbs = value - fromMin;
            float fromMaxAbs = fromMax - fromMin;

            float normal = fromAbs / fromMaxAbs;

            float toMaxAbs = toMax - toMin;
            float toAbs = toMaxAbs * normal;

            float to = toAbs + toMin;

            return to;
        }
    }
}
