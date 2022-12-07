namespace Volt
{
    public struct Quaternion
    {
        public float x;
        public float y;
        public float z;
        public float w;
        public static Quaternion Identity => new Quaternion(1f, 0f, 0f, 0f);

        public Quaternion(float eulerX, float eulerY, float eulerZ) : this()
        {
        }

        public Quaternion(float aW, float aX, float aY, float aZ) : this()
        {
            x = aX;
            y = aY;
            z = aZ;
            w = aW;
        }
    }
}
