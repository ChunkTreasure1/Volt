using System.Linq;
using System.Runtime.InteropServices;
using System.Security.Permissions;

namespace Volt
{
    public delegate void OnInputEnter();

    [EngineScript]
    public class TextInput : Script
    {
        public event OnInputEnter OnInputEnterEvent;

        public Font Font = null;
        public string Text = "...";
        public Color TextColor = Color.One;
        public uint MaxCharacters = 20;
        public Vector2 TextSize = new Vector2(10);
        public float MaxWidth = 100f;

        public Vector2 InputBounds = new Vector2(100, 50);
        public Vector3 InputOffset = new Vector3(0, 0, 10);

        public Color BackgroundNormalColor = Color.One;
        private Color currentColor;
        public Texture MainTexture;
        private Texture currentTexture;

        public bool Disabled = false;
        public bool Activated = false;

        private Vector2 TextScale;
        private bool isHovered;
        private bool isPressed;

        private Vector2 minBounds;
        private Vector2 maxBounds;

        private float backspaceCooldown = 0.05f;
        private bool backspaceAllowed = true;

        private void OnCreate()
        {
            TextScale = TextSize * entity.scale.XY;

            Vector2 colliderScale = InputBounds * entity.scale.XY;

            minBounds = new Vector2((entity.position.x + InputBounds.x) - ((colliderScale.x)), (entity.position.y + InputOffset.y) - ((colliderScale.y)));
            maxBounds = new Vector2((entity.position.x + InputBounds.x) + ((colliderScale.x)), (entity.position.y + InputOffset.y) + ((colliderScale.y)));
            currentColor = BackgroundNormalColor;
            currentTexture = MainTexture;
        }

        private void OnRenderUI()
        {
            UIRenderer.DrawSprite(currentTexture, entity.position + InputOffset, InputBounds * entity.scale.XY, entity.rotation.z, currentColor.AsVector4());
            UIRenderer.DrawString(Text, Font, entity.position, TextScale, entity.rotation.z, MaxWidth, TextColor.AsVector4());
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

                    if (Input.IsMouseReleased(MouseButton.Left))
                    {
                        Release();
                    }

                }

                if (isHovered)
                {
                    if (!isPressed && Input.IsMousePressed(MouseButton.Left))
                    {
                        Click();
                    }
                }
            }

            if (Activated)
            {
                backspaceCooldown -= Time.deltaTime;
                if (backspaceCooldown < 0)
                {
                    backspaceAllowed = true;
                    backspaceCooldown = 0.05f;
                }
                else
                {
                    backspaceAllowed = false;
                }

                CheckInput();
            }
        }

        private void OnEnterInput()
        {
            isPressed = false;
            Activated = false;

            OnInputEnterEvent?.Invoke();
        }

        private void OnEnterHovered()
        {
            isHovered = true;
        }
        private void OnExitHovered()
        {
            isHovered = false;
        }

        public void Click()
        {
            isPressed = true;
            Activated = true;
        }

        private void Release()
        {
            isPressed = false;
            Activated = false;
        }

        private void CheckInput()
        {
            uint[] allKeyPressed = Input.GetAllKeyPressed();

            string currentText = Text;

            if(Input.IsKeyDown(KeyCode.Backspace) && backspaceAllowed)
            {
                if (currentText == string.Empty) { return; }
                currentText = currentText.Remove(currentText.Length - 1, 1);
                Text = currentText;
                return;
            }

            foreach (uint key in allKeyPressed)
            {
                Log.Info(((KeyCode)key).ToString());

                //if ((KeyCode)key == KeyCode.Backspace )
                //{
                //    if(currentText == string.Empty) { return; }
                //    currentText = currentText.Remove(currentText.Length - 1, 1) ;
                //    Text = currentText;
                //    return;
                //}

                if((KeyCode)key == KeyCode.Enter)
                {
                    OnEnterInput();
                    return;
                }

                if ((KeyCode)key == KeyCode.Escape)
                {
                    Release();
                    return;
                }

                if(currentText.Length == MaxCharacters) { return; }

                if(key >= 48 && key <= 57)
                {
                    currentText += ((KeyCode)key).ToString().Last();
                }

                if (key == 46)
                {
                    if (Input.IsKeyDown(KeyCode.Left_Shift))
                    {
                        currentText += ":";
                    }
                    else
                    {
                        currentText += ".";
                    }
                }

                if(key >= 320 && key <= 329)
                {
                    currentText += ((KeyCode)key).ToString().Last();
                }

                if (key >= 65 && key <= 90)
                {
                    currentText += ((KeyCode)key).ToString();
                }

            }

            if(Input.IsKeyDown(KeyCode.Left_Control) && Input.IsKeyPressed(KeyCode.V))
            {
                
            }

            Text = currentText;

        }
    }
}
