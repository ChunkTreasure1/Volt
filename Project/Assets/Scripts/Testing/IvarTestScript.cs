using Volt;

namespace Project
{
    public class IvarTestScript : Script
    {
        public Entity testEntity;
        public Scene loadScene;
        public Prefab prefab;

        private void OnCreate()
        {
        }

        public void CallNetFunc(string funcName, uint entId, params object[] args)
        {
            var bytes = ArgumentPacker.PackArgs(args);
        }

        private void OnUpdate(float deltaTime)
        {
            if (Input.IsKeyPressed(KeyCode.M)) 
            {
                Scene.Load(loadScene);
            }

            if (Input.IsKeyPressed(KeyCode.P))
            {
                Entity.Create("Test");
            }
        }

        private void OnRenderUI()
        {
            //UIRenderer.DrawString(test, font, new Vector3(1920f / 2f, 1080f / 2f, 10f), new Vector2(100f), 0f, 100f, new Vector4(1f));
            //UIRenderer.DrawSprite(texture, new Vector3(1920f / 2f, 1080f / 2f, 10f), new Vector2(100f), 0f, new Vector4(1f));
        }
    }
}
