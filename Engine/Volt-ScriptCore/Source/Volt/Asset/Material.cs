namespace Volt
{
    public class Material : Asset
    {
        public Material() { }
        public Material(AssetHandle handle) : base(handle) { }

        public Vector4 color
        {
            set { InternalCalls.Material_SetColor(handle, ref value); }
            get
            {
                InternalCalls.Material_GetColor(handle, out Vector4 color);
                return color;
            }
        }

        public Vector3 emissiveColor
        {
            set { InternalCalls.Material_SetEmissiveColor(handle, ref value); }
            get
            {
                InternalCalls.Material_GetEmissiveColor(handle, out Vector3 color);
                return color;
            }
        }

        public float emissiveStrength
        {
            set { InternalCalls.Material_SetEmissiveStrength(handle, value); }
            get
            {
                return InternalCalls.Material_GetEmissiveStrength(handle);
            }
        }

        public float roughness
        {
            set { InternalCalls.Material_SetRoughness(handle, value); }
            get
            {
                return InternalCalls.Material_GetRoughness(handle);
            }
        }

        public float metalness
        {
            set { InternalCalls.Material_SetMetalness(handle, value); }
            get
            {
                return InternalCalls.Material_GetMetalness(handle);
            }
        }

        public float normalStrength
        {
            set { InternalCalls.Material_SetNormalStrength(handle, value); }
            get
            {
                return InternalCalls.Material_GetNormalStrength(handle);
            }
        }

        public void SetBool(string name, bool value) { InternalCalls.Material_SetBool(handle, name, value); }
        public void SetInt(string name, int value) { InternalCalls.Material_SetInt(handle, name, value); }
    
        public void SetUInt(string name, uint value) { InternalCalls.Material_SetUInt(handle, name, value); }
    
        public void SetFloat(string name, float value) { InternalCalls.Material_SetFloat(handle, name, value); }
        public void SetFloat2(string name, Vector2 value) { InternalCalls.Material_SetFloat2(handle, name, ref value); }
        public void SetFloat3(string name, Vector3 value) { InternalCalls.Material_SetFloat3(handle, name, ref value); }
        public void SetFloat4(string name, Vector4 value) { InternalCalls.Material_SetFloat4(handle, name, ref value); }
    
        public Material CreateCopy()
        {
            AssetHandle newHandle = InternalCalls.Material_CreateCopy(handle);
            if (newHandle == 0)
            {
                return null;
            }

            return new Material(newHandle);
        }
    }
}
