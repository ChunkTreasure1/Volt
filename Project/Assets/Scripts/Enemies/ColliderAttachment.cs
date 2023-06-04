using System.Windows.Forms.DataVisualization.Charting;
using Volt;

namespace Project
{
    public enum eBodyPart : byte
    {
        Head,
        Body,
        Leg,
        Other
    }

    public class ColliderAttachment : Script
    {
        public eBodyPart BodyPart;

        private Entity myMainEnemyActor;
        private Entity myControllerActor;
        private AnimationControllerComponent myAnimationControllerComponent;

        private void OnCreate()
        {
            myControllerActor = FindControllerActor(entity);

            if (myControllerActor != null)
            {
                if (myControllerActor.HasComponent<AnimationControllerComponent>())
                {
                    myAnimationControllerComponent = myControllerActor?.GetComponent<AnimationControllerComponent>();
                    myAnimationControllerComponent.controller?.AttachEntity(entity.name, entity);
                }
            }
            else
            {
                Log.Warning("Cant find animation controller for joint attachments on enemy!");
            }
        }

        private void OnDestroy()
        {
            myAnimationControllerComponent.controller?.DetachEntity(entity);
        }

        private Entity FindControllerActor(Entity ent)
        {
            if(ent.parent.Id != 0)
            {
                return FindControllerActor(ent.parent);
            }

            myMainEnemyActor = ent;

            foreach (Entity child in ent.children)
            {
                if (child.HasComponent<AnimationControllerComponent>())
                {
                    return child;
                }
            }

            return null;
        }
    }
}
