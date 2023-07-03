using System;
using System.Collections.Generic;
using System.Diagnostics.SymbolStore;
using System.Linq;
using System.Linq.Expressions;
using System.Text;
using System.Threading.Tasks;

namespace Volt
{
    public delegate void OnDrag();
    [EngineScript]
    internal class Slider : Script
    {
        public event OnDrag OnDrag;
        public event OnRelease OnRelease;
        public event OnClick OnClick;
        public event OnHover OnHover;
        public event OnExitHover OnExitHover;

        public Texture Handle = null;
        public Texture Background = null;

        public Texture HighlightedTexture = null;
        public Texture PressedTexture = null;
        public Texture DisabledTexture = null;

        public Vector2 BackgroundBounds = new Vector2(256, 16);
        public Vector2 HandleBounds = new Vector2(32, 32);

        public bool Disabled = false;
        public bool LeftToRight = true;
        public bool WholeNumbers = false;

        public float DragSpeed = 1000;
        public float MinValue = 0;
        public float MaxValue = 100;
        public float Value = 0;

        private float minPos;
        private float maxPos;

        private bool IsHovered = false;
        private bool IsPressed = false;

        private Vector3 HandlePos = new Vector3(0, 0, 0);
        private Vector2 minBounds = new Vector2(0, 0);
        private Vector2 maxBounds = new Vector2(0, 0);

        private Texture currentTexture;

        private void OnAwake()
        {
            Vector2 colliderScale = HandleBounds * entity.scale.XY;

            minBounds = new Vector2((entity.position.x) - ((colliderScale.x)), (entity.position.y) - ((colliderScale.y)));
            maxBounds = new Vector2((entity.position.x) + ((colliderScale.x)), (entity.position.y) + ((colliderScale.y)));
            currentTexture = Handle;
        }

        private void OnUpdate(float deltaTime)
        {
            if (!Disabled)
            {
                Vector2 colliderScale = HandleBounds * entity.scale.XY;

                minBounds = new Vector2((entity.position.x + HandlePos.x) - ((colliderScale.x)), (entity.position.y + HandlePos.y) - ((colliderScale.y)));
                maxBounds = new Vector2((entity.position.x + HandlePos.x) + ((colliderScale.x)), (entity.position.y + HandlePos.y) + ((colliderScale.y)));

                float mousePosX = Mathf.Remap(Input.GetMousePosition().x, 0, Window.GetWidth(), (-1920 / 2), 1920 / 2);
                float mousePosY = Mathf.Remap(Input.GetMousePosition().y, 0, Window.GetHeight(), (1080 / 2), -1080 / 2);

                if (mousePosX > minBounds.x && mousePosX < maxBounds.x &&
                    mousePosY > minBounds.y && mousePosY < maxBounds.y)
                {
                    if (!IsHovered)
                    {
                        OnEnterHovered();
                    }
                }
                else
                {
                    if (IsHovered)
                    {
                        OnExitHovered();
                    }
                }

                if (IsHovered)
                {
                    if (!IsPressed && Input.IsMousePressed(MouseButton.Left))
                    {
                        Click();
                    }
                }
                if (IsPressed && Input.IsMouseReleased(MouseButton.Left))
                {
                    Release();
                }

                maxPos = entity.position.x + (BackgroundBounds.x / 2) + (HandleBounds.x);
                minPos = entity.position.x - (BackgroundBounds.x / 2) - (HandleBounds.x);

                Vector2 mouseDirr = new Vector2(mousePosX, mousePosY) - HandlePos.XY;
                mouseDirr.Normalize();
                if (IsPressed)
                {
                    if (mouseDirr.x != 0)
                    {
                        OnDrag?.Invoke();
                    }
                    if (mouseDirr.x > 0 && HandlePos.x < maxPos)
                    {
                        HandlePos.x += mouseDirr.x * DragSpeed * deltaTime;
                        if (HandlePos.x > maxPos)
                        {
                            HandlePos.x = maxPos;
                        }
                    }
                    else if (mouseDirr.x < 0 && HandlePos.x > minPos)
                    {
                        HandlePos.x += mouseDirr.x * DragSpeed * deltaTime;
                        if (HandlePos.x < minPos)
                        {
                            HandlePos.x = minPos;
                        }
                    }
                }
                Value = (HandlePos.x - minPos) / (maxPos - minPos);
                if (!LeftToRight)
                {
                    Value = 1 - Value;
                }

                Value = (MaxValue - MinValue) * Value + MinValue;
                if (WholeNumbers)
                {
                    Value = (int)Value;
                }
            }
        }

        private void OnRenderUI()
        {
            if (Background.IsValid())
            {
                UIRenderer.DrawSprite(Background, entity.position, entity.scale.XY * BackgroundBounds, entity.rotation.z, Color.One.AsVector4());
            }
            else
            {
                UIRenderer.DrawSprite(entity.position, entity.scale.XY * BackgroundBounds, entity.rotation.z, Color.One.AsVector4());
            }
            if (currentTexture.IsValid())
            {
                UIRenderer.DrawSprite(currentTexture, entity.position + HandlePos, entity.scale.XY * HandleBounds, entity.rotation.z, Color.One.AsVector4());
            }
            else
            {
                UIRenderer.DrawSprite(entity.position + HandlePos, entity.scale.XY * HandleBounds, entity.rotation.z, new Vector4(0.5f, 1, 1, 1));
            }
        }

        private void OnEnterHovered()
        {
            IsHovered = true;
            if (HighlightedTexture.IsValid())
            {
                currentTexture = HighlightedTexture;
            }
            else
            {
                currentTexture = Handle;
            }

            if (OnHover != null)
            {
                OnHover.Invoke();
            }
        }
        private void OnExitHovered()
        {
            IsHovered = false;
            if (Handle.IsValid())
            {
                currentTexture = Handle;
            }
            else
            {
                currentTexture = Handle;
            }

            OnExitHover?.Invoke();
        }
        private void Click()
        {
            IsPressed = true;
            if (PressedTexture.IsValid())
            {
                currentTexture = PressedTexture;
            }
            else
            {
                currentTexture = Handle;
            }

            OnClick?.Invoke();

        }
        private void Release()
        {
            IsPressed = false;
            if (HighlightedTexture.IsValid())
            {
                currentTexture = HighlightedTexture;
            }
            else
            {
                currentTexture = Handle;
            }

            OnRelease?.Invoke();
        }
    }
}
