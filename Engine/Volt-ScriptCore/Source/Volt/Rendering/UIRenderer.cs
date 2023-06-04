namespace Volt
{
    public static class UIRenderer
    {
        public static void SetViewport(float x, float y, float width, float height)
        {
            InternalCalls.UIRenderer_SetViewport(x, y, width, height);
        }

        public static void SetScissor(int x, int y, uint width, uint height)
        {
            InternalCalls.UIRenderer_SetScissor(x, y, width, height);
        }

        public static void DrawSprite(Vector3 position, Vector2 scale, float rotation, Vector4 color, Vector2 offset = default)
        {
            InternalCalls.UIRenderer_DrawSprite(ref position, ref scale, rotation, ref color, ref offset);
        }

        public static void DrawSprite(Texture texture, Vector3 position, Vector2 scale, float rotation, Vector4 color = default, Vector2 offset = default)
        {
            InternalCalls.UIRenderer_DrawSpriteTexture(texture.handle, ref position, ref scale, rotation, ref color, ref offset);
        }

        public static void DrawString(string text, Font font, Vector3 position, Vector2 scale, float rotation, float maxWidth, Vector4 color) 
        {
            InternalCalls.UIRenderer_DrawString(text, font.handle, ref position, ref scale, rotation, maxWidth, ref color);
        }
    }
}
