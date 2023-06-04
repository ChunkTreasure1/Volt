using System.Linq;
using System.Linq.Expressions;

namespace Volt
{
    [EngineScript]
    public class Text : Script
    {
        public Font Font = null;
        public string TextString = "Text";
        public Color TextColor = Color.One;
        public Vector3 TextOffset = new Vector3(0, 0, 15);
        public Vector2 TextSize = new Vector2(10);
        public float MaxWidth = 100f;

        public bool IsCentered = false;

        private Vector2 TextScale;

        private void OnAwake()
        {
            
        }

        private void OnCreate()
        {
            TextScale = TextSize * entity.scale.XY;
            if (IsCentered)
            {
                TextOffset = new Vector3((TextString.Count() * (TextScale.x / 2)) / 2, (TextScale.y / 2), TextOffset.z);
                TextOffset.x *= -1;
                TextOffset.y *= -1;
                Log.Info("TextOffSet:" + TextOffset.x.ToString() + "," + TextOffset.y.ToString());
            }
        }

        private void OnRenderUI()
        {
            Vector3 tempOffset = TextOffset;
            if (IsCentered && Font != null && Font.IsValid())
            {
                tempOffset -= new Vector3(Font.GetStringWidth(TextString, TextScale, MaxWidth) / 2f, 0f, 0f);
            }

            UIRenderer.DrawString(TextString, Font, entity.position + tempOffset, TextScale, entity.rotation.z, MaxWidth, TextColor.AsVector4());
        }
    }
}
