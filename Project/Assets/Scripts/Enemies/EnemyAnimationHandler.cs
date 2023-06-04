using Volt;

namespace Project
{
    public enum eEnemyAnimation : byte
    {
        Walk,
        Run,
        Attack,
        BitePlank
    }

    public class EnemyAnimationHandler : Script
    {
        AnimationControllerComponent myAnimationController;

        private void OnCreate()
        {
            if (entity.children[0].HasComponent<AnimationControllerComponent>())
            {
                myAnimationController = entity.children[0].GetComponent<AnimationControllerComponent>();
            }
        }

        public void SetAnimation(eEnemyAnimation animation, bool active)
        {
            switch (animation)
            {
                case eEnemyAnimation.Walk:
                    myAnimationController.controller.SetParameter("Run", false);
                    break;
                case eEnemyAnimation.Run:
                    myAnimationController.controller.SetParameter("Run", true);
                    break;
                case eEnemyAnimation.Attack:
                    myAnimationController.controller.SetParameter("Attack", active);
                    break;
                case eEnemyAnimation.BitePlank:
                    myAnimationController.controller.SetParameter("BitePlanks", active);
                    break;
            }
        }

        public void OnDeath()
        {
            int randomDeath = Volt.Random.Range(0, 3);
            myAnimationController?.controller?.SetParameter("DeathType", randomDeath);
            myAnimationController?.controller?.SetParameter("Die", true);            
        }
    }
}
