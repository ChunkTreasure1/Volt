namespace Volt
{
    public enum ShadowResolutionSetting : uint
    {
        Low = 0,
        Medium = 1,
        High = 2,
    }

    public enum AntiAliasingSetting : uint
    {
        FXAA = 0,
        TAA = 1,
    }

    public enum AOQualitySetting : uint
    {
        Low = 0,
        Medium = 1,
        High = 2,
        Ultra = 3
    }

    public struct RendererSettings
    {
        public ShadowResolutionSetting shadowResolution;
        public AntiAliasingSetting antiAliasing;
        public AOQualitySetting aOQuality;

        public float renderScale;
        
        ///// Render Techniques /////
        public bool enableShadows;
        public bool enableAO;
        public bool enableBloom;
        public bool enableAntiAliasing;
        public bool enableUI;
        public bool enableSkybox;
        public bool enablePostProcessing;
        public bool enableRayTracing;
        public bool enableVolumetricFog;

        ///// Debug/Editor /////
        public bool enableDebugRenderer;
        public bool enableOutline;
        public bool enableIDRendering;
        public bool enableGrid;

        public static RendererSettings GetDefault()
        {
            RendererSettings settings = new RendererSettings();
            settings.shadowResolution = ShadowResolutionSetting.Medium;
            settings.antiAliasing = AntiAliasingSetting.FXAA;
            settings.aOQuality = AOQualitySetting.High;
            settings.renderScale = 1f;

            settings.enableShadows = true;
            settings.enableAO = true;
            settings.enableBloom = true;
            settings.enableAntiAliasing = false;
            settings.enableUI = true;
            settings.enableSkybox = true;
            settings.enablePostProcessing = false;
            settings.enableRayTracing = false;
            settings.enableVolumetricFog = true;

            settings.enableDebugRenderer = false;
            settings.enableOutline = false;
            settings.enableIDRendering = false;
            settings.enableGrid = false;

            return settings;
        }

    }

    public static class Renderer
    {
        public static void SetRenderScale(float renderScale)
        {
            InternalCalls.Renderer_SetRenderScale(renderScale);
        }

        public static void SetRendererSettings(RendererSettings settings)
        {
            InternalCalls.Renderer_SetRendererSettings(ref settings);
        }
    }
}
