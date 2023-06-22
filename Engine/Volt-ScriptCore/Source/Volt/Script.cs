using System;
using System.Runtime.Remoting.Metadata.W3cXsd2001;

namespace Volt
{
    public class Script
    {
        public readonly Entity entity;
        public readonly ulong scriptId;

        public Script() { }

        internal Script(Entity aEntity, ulong aScriptId)
        {
            entity = aEntity;
            scriptId = aScriptId;
        }

        public void Notify(string fieldName)
        {
            NetScene.Notify(entity.Id, fieldName);
        }

        // Available Functions.

        // private void OnAwake() { }
        // private void OnCreate() { }
        // private void OnDestroy() { }
        // private void OnUpdate(float deltaTime) { }
        // private void OnFixedUpdate(float deltaTime) { }

        // private void OnCollisionEnter(Entity other) { }
        // private void OnCollisionExit(Entity other) { }
        // private void OnTriggerEnter(Entity other) { }
        // private void OnTriggerExit(Entity other) { }

        // private void OnEnable() { }
        // private void OnDisable() { }
    }
}
