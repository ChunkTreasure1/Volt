using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Volt
{
    public delegate void OnValueChanged();
    public delegate void OnValueTrue();
    public delegate void OnValueFalse();


    [EngineScript]
    public class CheckBox : Script
    {

        public event OnValueChanged OnChanged;
        public event OnValueTrue OnTrue;
        public event OnValueFalse OnFalse;

        public bool CheckBoxValue;
        public Texture UnCheckedTexture;
        public Texture CheckedTexture;
        public Texture DisabledTexture;

        public Vector3 Offset = new Vector3(0, 0, 10);
        public Vector2 TextureSize = new Vector2(64, 64);
        public Color Color = Color.One;

        public bool IsDisabled;

        private Texture CurrentTexture;
        private Vector2 minBounds;
        private Vector2 maxBounds;

        private void OnAwake()
        {
            Vector2 colliderScale = TextureSize * entity.scale.XY;

            minBounds = new Vector2((entity.position.x + Offset.x) - ((colliderScale.x)), (entity.position.y + Offset.y) - ((colliderScale.y)));
            maxBounds = new Vector2((entity.position.x + Offset.x) + ((colliderScale.x)), (entity.position.y + Offset.y) + ((colliderScale.y)));

            if (OnChanged != null)
            {
                OnChanged?.Invoke();
            }

            if (IsDisabled)
            {
                CurrentTexture = DisabledTexture;
            }
            else
            {
                if (CheckBoxValue)
                {
                    if (OnTrue != null)
                    {
                        OnTrue?.Invoke();
                    }
                    CurrentTexture = CheckedTexture;
                }
                else
                {
                    if (OnFalse != null)
                    {
                        OnFalse?.Invoke();
                    }
                    CurrentTexture = UnCheckedTexture;
                }
            }
        }


        private void OnRenderUI()
        {
            if (CurrentTexture.IsValid())
            {
                UIRenderer.DrawSprite(CurrentTexture, entity.position + Offset, TextureSize * entity.scale.XY, entity.rotation.z, Color.AsVector4());
            }
        }


        private void OnUpdate(float deltaTime)
        {
            if (!IsDisabled)
            {
                float mousePosX = Mathf.Remap(Input.GetMousePosition().x, 0, Window.GetWidth(), (-1920 / 2), 1920 / 2);
                float mousePosY = Mathf.Remap(Input.GetMousePosition().y, 0, Window.GetHeight(), (1080 / 2), (-1080 / 2));

                if (mousePosX > minBounds.x && mousePosX < maxBounds.x &&
                    mousePosY > minBounds.y && mousePosY < maxBounds.y)
                {
                    if (Input.IsMousePressed(MouseButton.Left))
                    {
                        OnChanged?.Invoke();
                        CheckBoxValue = !CheckBoxValue;

                        if (CheckBoxValue)
                        {
                            OnTrue?.Invoke();
                            CurrentTexture = CheckedTexture;
                        }
                        else
                        {
                            OnFalse?.Invoke();
                            CurrentTexture = UnCheckedTexture;
                        }
                    }
                }
            }
        }
    }
}
