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
        public bool IsRightAligned = false;
        public bool IsVerticalCentered = false;

        private Vector2 TextScale;

        private void OnAwake()
        {
            
        }

        private void OnCreate()
        {
        }

        private void OnRenderUI()
        {
            TextScale = TextSize * entity.scale.XY;

            Vector2 tempOffset = Vector2.Zero;
            if (IsCentered && Font != null && Font.IsValid())
            {
                tempOffset += new Vector2(Font.GetStringWidth(TextString, TextScale, MaxWidth) / 2f, 0f);
            }
            else if (IsRightAligned && Font != null && Font.IsValid())
            {
                tempOffset += new Vector2(Font.GetStringWidth(TextString, TextScale, MaxWidth), 0f);
            }

            if (IsVerticalCentered)
            {
                tempOffset += new Vector2(0f, Font.GetStringHeight(TextString, TextScale, MaxWidth) / 2f);
            }

            UIRenderer.DrawString(TextString, Font, entity.position + TextOffset, TextScale, entity.rotation.z, MaxWidth, TextColor.AsVector4(), tempOffset);
        }
        public float GetStringWidth()
        {
            return Font.GetStringWidth(TextString, TextScale, MaxWidth);
        }

        public float GetStringHeight()
        {
            return Font.GetStringHeight(TextString, TextScale, MaxWidth);
        }
    }
}
