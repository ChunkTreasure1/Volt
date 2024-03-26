namespace Volt
{
    public class SpotLightComponent : Component
    {
        public override string GUID { get => "{D35F915F-53E5-4E15-AE5B-769F4D79B6F8}"; }
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
        public override string GUID { get => "{A30A8848-A30B-41DD-80F9-4E163C01ABC2}"; }

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
