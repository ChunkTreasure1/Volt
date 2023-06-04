using System;

namespace Volt
{
    public struct BoundingSphere
    {
        public Vector3 center;
        public float radius;
    }

    public class Mesh : Asset
    {
        public Mesh() { }
        public Mesh(AssetHandle handle) : base(handle) { }

        public Material material
        {
            get
            {
                return AssetManager.GetAsset<Material>(InternalCalls.Mesh_GetMaterial(handle));
            }

            private set
            {
            }
        }
    }
}
