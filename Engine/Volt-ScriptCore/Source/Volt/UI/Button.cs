using System.Linq.Expressions;

namespace Volt
{
    public delegate void OnClick();
    public delegate void OnRelease();
    public delegate void OnHover();
    public delegate void OnExitHover();

    [EngineScript]
    public class Button : Script
    {
        public event OnClick OnClick;
        public event OnRelease OnRelease;
        public event OnHover OnHover;
        public event OnExitHover OnExitHover;

        public Vector2 ButtonBounds = new Vector2(100, 50);

        public Texture MainTexture;
        public Color NormalColor = Color.One;
        public Color HightlghtedColor = new Color(0.9f, 0.9f, 0.9f, 1);
        public Color PressedColor = new Color(0.3f, 0.3f, 0.3f, 1);
        public Color DisabledColor = new Color(0.5f, 0.5f, 0.5f, 1);
        public Vector3 ButtonOffset = new Vector3(0, 0, 10);

        public Texture PressedTexture;
        public Texture HighlightedTexture;
        public Texture DisabledTexture;

        public bool Disabled = false;

        private Color currentColor;
        private Texture currentTexture;

        private bool isHovered;
        private bool isPressed;

        private Vector2 minBounds;
        private Vector2 maxBounds;

        private void OnAwake()
        {
            Vector2 colliderScale = ButtonBounds * entity.scale.XY;

            currentTexture = MainTexture;

            minBounds = new Vector2((entity.position.x + ButtonOffset.x) - ((colliderScale.x) ), (entity.position.y + ButtonOffset.y) - ((colliderScale.y)));
            maxBounds = new Vector2((entity.position.x + ButtonOffset.x) + ((colliderScale.x) ), (entity.position.y + ButtonOffset.y) + ((colliderScale.y)));
            currentColor = NormalColor;

            OnClick += new OnClick(() => { Log.Info("Button clicked"); });

            if (Disabled)
            {
                if (DisabledTexture.IsValid())
                {
                    currentTexture = DisabledTexture;
                }
                else
                {
                    currentColor = DisabledColor;
                }
            }
        }

        private void OnRenderUI()
        {
            UIRenderer.DrawSprite(currentTexture, entity.position + ButtonOffset, ButtonBounds * entity.scale.XY, entity.rotation.z, currentColor.AsVector4());
        }

        private void OnUpdate(float deltaTime)
        {
            if (!Disabled)
            {
                float mousePosX = Mathf.Remap(Input.GetMousePosition().x, 0, Window.GetWidth(), (-1920 / 2), 1920 / 2);
                float mousePosY = Mathf.Remap(Input.GetMousePosition().y, 0, Window.GetHeight(), (1080 / 2), -1080 / 2);

                if (mousePosX > minBounds.x && mousePosX < maxBounds.x &&
                    mousePosY > minBounds.y && mousePosY < maxBounds.y)
                {
                    if (!isHovered)
                    {
                        OnEnterHovered();
                    }
                }
                else
                {
                    if (isHovered)
                    {
                        OnExitHovered();
                    }
                }

                if (isHovered)
                {
                    if (!isPressed && Input.IsMousePressed(MouseButton.Left))
                    {
                        Click();
                    }
                }
                if (isPressed && Input.IsMouseReleased(MouseButton.Left))
                {
                    Release();
                }
            }
        }

        private void OnEnterHovered()
        {
            isHovered = true;
            if (HighlightedTexture.IsValid())
            {
                currentColor = NormalColor;
                currentTexture = HighlightedTexture;
            }
            else
            {
                currentTexture = MainTexture;

                currentColor = HightlghtedColor;
            }

            if (OnHover != null)
            {
                OnHover.Invoke();
            }
        }
        private void OnExitHovered()
        {
            isHovered = false;
            if (MainTexture.IsValid())
            {
                currentColor = NormalColor;
                currentTexture = MainTexture;
            }
            else
            {
                currentTexture = MainTexture;

                currentColor = NormalColor;
            }
            if (OnExitHover != null)
            {
                OnExitHover.Invoke();
            }
        }

        public void Click()
        {
            isPressed = true;
            if (PressedTexture.IsValid())
            {
                currentColor = NormalColor;
                currentTexture = PressedTexture;
            }
            else
            {
                currentTexture = MainTexture;

                currentColor = PressedColor;
            }
            if (OnClick != null)
            {
                OnClick.Invoke();
            }
        }

        private void Release()
        {
            isPressed = false;
            if (HighlightedTexture.IsValid())
            {
                currentColor = NormalColor;
                currentTexture = HighlightedTexture;
            }
            else
            {
                currentTexture = MainTexture;
                currentColor = HightlghtedColor;
            }
            if (OnRelease != null)
            {
                OnRelease.Invoke();
            }
        }

    }
}
