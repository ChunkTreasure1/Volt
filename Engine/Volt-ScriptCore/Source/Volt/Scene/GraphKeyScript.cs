namespace Volt
{
    public class GraphKeyScript
    {
        private Entity myEntity;

        public GraphKeyScript(Entity entity)
        {
            this.myEntity = entity;
        }

        public void DispatchEvent(string eventName)
        {
            InternalCalls.GraphKey_DispatchEvent(myEntity.Id, eventName);
        }

        public void SetParameter(string name, float value)
        {
            InternalCalls.VisualScriptingComponent_SetParameterFloat(myEntity.Id, name, value);
        }

        public void SetParameter(string name, int value)
        {
            InternalCalls.VisualScriptingComponent_SetParameterInt(myEntity.Id, name, value);
        }

        public void SetParameter(string name, bool value)
        {
            InternalCalls.VisualScriptingComponent_SetParameterBool(myEntity.Id, name, value);
        }

        public void SetParameter(string name, Vector3 value)
        {
            InternalCalls.VisualScriptingComponent_SetParameterVector3(myEntity.Id, name, ref value);
        }

        public void SetParameter(string name, string value)
        {
            InternalCalls.VisualScriptingComponent_SetParameterString(myEntity.Id, name, ref value);
        }
    }
}
