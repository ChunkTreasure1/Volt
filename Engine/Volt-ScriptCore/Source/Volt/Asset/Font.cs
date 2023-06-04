
namespace Volt
{
    public class Font : Asset
    {
        public Font() { }
        public Font(AssetHandle handle) : base(handle) { }

        public float GetStringWidth(string text, Vector2 scale, float maxWidth)
        {
            return InternalCalls.Font_GetStringWidth(handle, text, ref scale, maxWidth);
        }
    }
}
