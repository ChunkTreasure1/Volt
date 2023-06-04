using System;

namespace Volt
{
    public class Prefab : Asset
    {
        public Prefab() { }
        public Prefab(AssetHandle handle) : base(handle) { }
        public Entity Instantiate()
        {
            object entity = InternalCalls.Entity_CreateNewEntityWithPrefab(handle);
            return entity as Entity;
        }
    }
}
