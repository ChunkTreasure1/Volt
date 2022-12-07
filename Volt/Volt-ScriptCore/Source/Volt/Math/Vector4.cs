namespace Volt
{
    public struct Vector4
    {
        public float x;
        public float y;
        public float z;
        public float w;

        public static Vector4 Zero => new Vector4(0f);
        public static Vector4 One => new Vector4(1f);

        public Vector4(float scalar) : this()
        {
            x = scalar;
            y = scalar;
            z = scalar;
            w = scalar;
        }

        public Vector4(float aX, float aY, float aZ, float aW) : this()
        {
            x = aX;
            y = aY;
            z = aZ;
            w = aW;
        }
    }
}
