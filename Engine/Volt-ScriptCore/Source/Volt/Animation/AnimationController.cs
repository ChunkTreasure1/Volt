namespace Volt
{
    public class AnimationController
    {
        private Entity myEntity;

        public AnimationController(Entity entity)
        {
            this.myEntity = entity;
        }

        public void SetParameter(string name, float value)
        {
            InternalCalls.AnimationControllerComponent_SetParameterFloat(myEntity.Id, name, value);
        }

        public void SetParameter(string name, int value)
        {
            InternalCalls.AnimationControllerComponent_SetParameterInt(myEntity.Id, name, value);
        }

        public void SetParameter(string name, bool value)
        {
            InternalCalls.AnimationControllerComponent_SetParameterBool(myEntity.Id, name, value);
        }

        public void SetParameter(string name, Vector3 value)
        {
            InternalCalls.AnimationControllerComponent_SetParameterVector3(myEntity.Id, name, ref value);
        }

        public void SetParameter(string name, string value)
        {
            InternalCalls.AnimationControllerComponent_SetParameterString(myEntity.Id, name, ref value);
        }

        public float GetParameterFloat(string name)
        {
            return InternalCalls.AnimationControllerComponent_GetParameterFloat(myEntity.Id, name);
        }

        public int GetParameterInt(string name)
        {
            return InternalCalls.AnimationControllerComponent_GetParameterInt(myEntity.Id, name);
        }

        public bool GetParameterBool(string name)
        {
            return InternalCalls.AnimationControllerComponent_GetParameterBool(myEntity.Id, name);
        }

        public Vector3 GetParameterVector3(string name)
        {
            return InternalCalls.AnimationControllerComponent_GetParameterVector3(myEntity.Id, name);
        }

        public string GetParameterString(string name)
        {
            return InternalCalls.AnimationControllerComponent_GetParameterString(myEntity.Id, name);
        }

        public void AttachEntity(string attachmentName,  Entity attachEntity)
        {
            InternalCalls.AnimationControllerComponent_AttachEntity(attachmentName, myEntity.Id, attachEntity.Id);
        }

        public void DetachEntity(Entity entity)
        {
            InternalCalls.AnimationControllerComponent_DetachEntity(myEntity.Id, entity.Id);
        }

        public void SetAnimationGraph(ulong handle)
        {
            InternalCalls.AnimationControllerComponent_SetController(myEntity.Id, handle);
        }
    }
}
