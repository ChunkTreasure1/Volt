namespace Volt
{
    public class SpotLightComponent : Component
    {
        public Vector3 color
        {
            get
            {
                InternalCalls.SpotlightComponent_GetColor(entity.Id, out Vector3 color);
                return color;
            }

            set
            {
                InternalCalls.SpotlightComponent_SetColor(entity.Id, ref value);
            }
        }

        public float intensity
        {
            get
            {
                float intensity = InternalCalls.SpotlightComponent_GetIntensity(entity.Id);
                return intensity;
            }

            set
            {
                InternalCalls.SpotlightComponent_SetIntensity(entity.Id, value);
            }
        }
    }

    public class PointLightComponent : Component
    {
        public Vector3 color
        {
            get
            {
                InternalCalls.PointlightComponent_GetColor(entity.Id, out Vector3 color);
                return color;
            }

            set
            {
                InternalCalls.PointlightComponent_SetColor(entity.Id, ref value);
            }
        }

        public float intensity
        {
            get
            {
                float intensity = InternalCalls.PointlightComponent_GetIntensity(entity.Id);
                return intensity;
            }

            set
            {
                InternalCalls.PointlightComponent_SetIntensity(entity.Id, value);
            }
        }
    }
}
