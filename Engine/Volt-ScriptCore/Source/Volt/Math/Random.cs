namespace Volt
{
    public static class Random
    {
        static System.Random myRandom = new System.Random();

        public static float Float()
        {
            return (float)myRandom.NextDouble();
        }

        public static Vector3 Vec3()
        {
            return new Vector3(Float(), Float(), Float());
        }

        public static double Double()
        {
            return myRandom.NextDouble();
        }

        public static float Range(float minValue, float maxValue)
        {
            return Float() * (maxValue - minValue) + minValue;
        }

        public static int Range(int minValue, int maxValue)
        {
            return myRandom.Next(minValue, maxValue);
        }
    }
}
