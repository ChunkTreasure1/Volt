namespace Volt
{
    public static class Interpolate
    {

        public static float Linear(float p1, float p2, float t)
        {
            if (t < 0.0f)
                return p1;
            else if (t > 1.0f)
                return p2;

            return p1 + ((p2 - p1) * t);
        }

        public static Vector3 Linear(Vector3 p1, Vector3 p2, float t)
        {
            if (t < 0.0f)
                return p1;
            else if (t > 1.0f)
                return p2;

            return p1 + ((p2 - p1) * t);
        }

        public static Vector3 EaseIn(Vector3 p1, Vector3 p2, float t)
        {
            return Linear(p1, p2, EaseIn_Time(t));
        }

        public static Vector3 EaseOut(Vector3 p1, Vector3 p2, float t)
        {
            return Linear(p1, p2, EaseOut_Time(t));
        }

        public static float EaseInOut(float a, float b, float t)
        {
            return Linear(a, b, EaseInOut_Time(t));
        }

        public static Vector3 EaseInOut(Vector3 p1, Vector3 p2, float t)
        {
            return Linear(p1, p2, EaseInOut_Time(t));
        }

        private static float EaseIn_Time(float t)
        {
            return Square(t);
        }

        private static float EaseOut_Time(float t)
        {
            return Invert(Square(Invert(t)));
        }

        private static float EaseInOut_Time(float t)
        {
            return Linear(EaseIn_Time(t), EaseOut_Time(t), t);
        }

        internal static float Invert(float x)
        {
            return 1.0f - x;
        }

        internal static float Square(float x)
        {
            return x * x;
        }

        internal static float BounceOut(float t, float b, float c, float d)
        {
            c -= b;
            float result = 0;
            if ((t /= d) < (1 / 2.75f))
            {
                result = c * (7.5625f * t * t) + b;
            }
            else if (t < (2 / 2.75f))
            {
                t -= (1.5f / 2.75f);
            }
            else if (t < (2.5 / 2.75))
            {
                t -= (2.25f / 2.75f);
                result = c * (7.5625f * (t) * t + .9375f) + b;
            }
            else if (t < (3 / 2.75))
            {
                t -= (2.625f / 2.75f);
                result = c * (7.5625f * (t) * t + .984375f) + b;
            }
            else if (t < (3.25 / 2.75))
            {
                t -= (3.125f / 2.75f);
                result = c * (7.5625f * (t) * t + .99609375f) + b;
            }

            return result;
        }
    }
}
