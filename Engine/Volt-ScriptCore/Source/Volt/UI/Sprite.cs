using System.Linq.Expressions;


namespace Volt
{
    [EngineScript]
    public class Sprite : Script
    {
        public Texture Texture = null;
        public Color Color = Color.One;
        public Vector3 OffSet = new Vector3(0, 0, 15);
        public Vector2 PixelSize = new Vector2(50, 50);

        private void OnAwake()
        {
        }

        private void OnRenderUI()
        {
            UIRenderer.DrawSprite(Texture, entity.position + OffSet, PixelSize * entity.scale.XY, entity.rotation.z, Color.AsVector4());
        }
    }
}
