namespace Volt
{
    public class Vision
    {
        public struct CameraShakeSettings
        {
            public Vector3 rotationAmount;
            public Vector3 translationAmount;

            public float rotationMagnitude;
            public float translationMagnitude;
            public float shakeTime;
        }

        public static Entity GetActiveCamera()
        {
            uint entId = InternalCalls.Vision_GetActiveCamera();

            if (entId == 0) { return null; }

            Entity ent = new Entity(entId);

            return ent;
        }

        public static void SetActiveCamera(uint id)
        {
            InternalCalls.Vision_SetActiveCamera(id);
        }

        public static void DoCameraShake(Entity ent, CameraShakeSettings setting)
        {
            InternalCalls.Vision_DoCameraShake(ent.Id, ref setting);
        }

        public static void SetCameraFollow(Entity cameraEnt, Entity followEnt)
        { 
            if(followEnt == null)
            {
                InternalCalls.Vision_SetCameraFollow(cameraEnt.Id, 0);
                return;
            }
            InternalCalls.Vision_SetCameraFollow(cameraEnt.Id, followEnt.Id);
        }

        public static void SetCameraLookAt(Entity cameraEnt, Entity lookatEnt)
        {
            if (lookatEnt == null)
            {
                InternalCalls.Vision_SetCameraLookAt(cameraEnt.Id, 0);
                return;
            }
            InternalCalls.Vision_SetCameraLookAt(cameraEnt.Id, lookatEnt.Id);
        }

        public static void SetCameraFocusPoint(Entity cameraEnt, Entity focusEnt)
        {
            if(focusEnt == null)
            {
                InternalCalls.Vision_SetCameraFocusPoint(cameraEnt.Id, 0);
                return;
            }
            InternalCalls.Vision_SetCameraFocusPoint(cameraEnt.Id, focusEnt.Id);
        }

        public static void SetCameraDamping(Entity cameraEnt, float dampAmount)
        {
            InternalCalls.Vision_SetCameraDampAmount(cameraEnt.Id, dampAmount);
        }

        public static void SetCameraFoV(Entity cameraEnt, float FoV)
        {
            InternalCalls.Vision_SetCameraFieldOfView(cameraEnt.Id, FoV);
        }

        public static void SetCameraLocked(Entity cameraEnt, bool locked)
        {
            InternalCalls.Vision_SetCameraLocked(cameraEnt.Id, locked);
        }

        public static void SetCameraMouseSensentivity(Entity cameraEnt, float mouseSens)
        {
            InternalCalls.Vision_SetCameraMouseSensentivity(cameraEnt.Id, mouseSens);
        }
    }
}
