namespace Volt
{
    [EngineScript]
    public class VideoPlayer : Script
    {
        public Color Color = Color.One;
        public Vector3 Offset = new Vector3(0, 0, 15);
        public Vector2 PixelSize = new Vector2(50, 50);
        public Video Video;
        public bool PlayOnAwake = false;

        private void OnAwake()
        {
            if (PlayOnAwake)
            {
                Play(false);
            }
        }

        public void Play(bool loop = false)
        {
            InternalCalls.VideoPlayer_Play(Video.handle, loop);
        }

        public void Stop()
        {
            InternalCalls.VideoPlayer_Stop(Video.handle);
        }

        public void Restart()
        {
            InternalCalls.VideoPlayer_Restart(Video.handle);
        }

        public void Pause()
        {
            InternalCalls.VideoPlayer_Pause(Video.handle);
        }

        private void OnUpdate(float deltaTime)
        {
            InternalCalls.VideoPlayer_Update(Video.handle, deltaTime);
        }

        private void OnRenderUI()
        {
            VideoStatus status = InternalCalls.VideoPlayer_GetStatus(Video.handle);
            if (status != VideoStatus.Stopped)
            {
                Vector2 offset = new Vector2(0f);
                Vector4 color = Color.AsVector4();
                Vector3 position = entity.position + Offset;
                Vector2 scale = PixelSize * entity.scale.XY;

                InternalCalls.VideoPlayer_DrawSpriteTexture(Video.handle, ref position, ref scale, entity.rotation.z, ref color, ref offset);
            }
        }
    }
}
