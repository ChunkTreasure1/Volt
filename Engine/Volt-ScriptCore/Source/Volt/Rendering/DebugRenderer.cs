namespace Volt
{
    public class DebugRenderer
    {
#if DIST
        public static void DrawLine(Vector3 startPosition, Vector3 endPosition) { }
        public static void DrawLine(Vector3 startPosition, Vector3 endPosition, Vector4 color) { }
        public static void DrawSprite(Vector3 position, Vector4 color) { }
        public static void DrawSprite(Vector3 position, Vector3 rotation, Vector3 scale, Vector4 color) { }
        public static void DrawText(string text, Vector3 position) { }
        public static void DrawText(string text, Vector3 position, Vector3 rotation, Vector3 scale) { }
        public static void DrawRay(Vector3 origin, Vector3 direction, float distance) { }
#else

        public static void DrawLine(Vector3 startPosition, Vector3 endPosition)
        {
            Vector4 color = new Vector4(1f);
            InternalCalls.DebugRenderer_DrawLine(ref startPosition, ref endPosition, ref color);
        }

        public static void DrawLine(Vector3 startPosition, Vector3 endPosition, Vector4 color)
        {
            InternalCalls.DebugRenderer_DrawLine(ref startPosition, ref endPosition, ref color);
        }

        public static void DrawSprite(Vector3 position, Vector4 color)
        {
            Vector3 rotation = new Vector3(0f);
            Vector3 scale = Vector3.One;

            InternalCalls.DebugRenderer_DrawSprite(ref position, ref rotation, ref scale, ref color);
        }

        public static void DrawSprite(Vector3 position, Vector3 rotation, Vector3 scale, Vector4 color)
        {
            Vector3 radRot = Mathf.Radians(rotation);
            InternalCalls.DebugRenderer_DrawSprite(ref position, ref radRot, ref scale, ref color);
        }

        public static void DrawText(string text, Vector3 position)
        {
            Vector3 rotation = new Vector3(0f);
            Vector3 scale = Vector3.One;

            InternalCalls.DebugRenderer_DrawText(ref text, ref position, ref rotation, ref scale);
        }

        public static void DrawText(string text, Vector3 position, Vector3 rotation, Vector3 scale)
        {
            Vector3 radRot = Mathf.Radians(rotation);
            InternalCalls.DebugRenderer_DrawText(ref text, ref position, ref radRot, ref scale);
        }

        public static void DrawRay(Vector3 origin, Vector3 direction, float distance)
        {
            Vector3 endPosition = direction.Normalized() * distance + origin;
            DrawLine(origin, endPosition);
        }
#endif
    }
}
