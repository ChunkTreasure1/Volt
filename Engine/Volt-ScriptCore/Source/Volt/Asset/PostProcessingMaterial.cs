
namespace Volt
{
    public class PostProcessingMaterial : Asset
    {
        public PostProcessingMaterial() { }
        public PostProcessingMaterial(AssetHandle handle) : base(handle) { }

        public void SetBool(string name, bool value) { InternalCalls.PostProcessingMaterial_SetBool(handle, name, value); }
        public void SetInt(string name, int value) { InternalCalls.PostProcessingMaterial_SetInt(handle, name, value); }

        public void SetUInt(string name, uint value) { InternalCalls.PostProcessingMaterial_SetUInt(handle, name, value); }

        public void SetFloat(string name, float value) { InternalCalls.PostProcessingMaterial_SetFloat(handle, name, value); }
        public void SetFloat2(string name, Vector2 value) { InternalCalls.PostProcessingMaterial_SetFloat2(handle, name, ref value); }
        public void SetFloat3(string name, Vector3 value) { InternalCalls.PostProcessingMaterial_SetFloat3(handle, name, ref value); }
        public void SetFloat4(string name, Vector4 value) { InternalCalls.PostProcessingMaterial_SetFloat4(handle, name, ref value); }
    }
}
