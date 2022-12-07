namespace Volt
{
    public struct Vector2
    {
        public float x;
        public float y;

        public static Vector2 Zero => new Vector2(0f);
        public static Vector2 One => new Vector2(1f);
    
        public Vector2(float scalar) : this()
        {
            x = scalar;
            y = scalar;
        }

        public Vector2(float aX, float aY) : this()
        {
            x = aX;
            y = aY;
        }
    }
}
