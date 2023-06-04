using System.ComponentModel;
using System.Drawing;
using System.EnterpriseServices;
using Volt;

namespace Project
{
    public enum ImapctType
    {
        Wall,
        Body
    }

    public class VFXManager : Script
    {
        #region Static Instance
        static VFXManager _instance;
        public static VFXManager Instance
        {
            get
            {
                if (_instance == null)
                {
                    Log.Warning("No Instance available.");
                }
                return _instance;
            }
        }
        private void OnAwake()
        {
            _instance = this;
        }

        #endregion

        //COMMON VFX
        public Prefab WallImpact;
        public Prefab BodyImpact;
        //COMMON VFX

        //PostProcess Effects
        public PostProcessingMaterial DamageIndicator;
        public PostProcessingMaterial BlinkEffect;
        public PostProcessingMaterial TakeDamageEffect;
        //PostProcess Effects

        //Blink Effect Vars
        bool blinkEffectActive = false;
        float blinkDuration = 4f;
        float blinkCurrentTime = 0f;
        float currentBlinkValue = 0f;
        //Blink Effect Vars

        //Take Damage Effect Vars
        float currentDamageValue = 0f;
        float disappearanceSpeed = 0.5f;
        float timeUntilRegen = 0f;
        //Take Damage Effect Vars

        private void OnUpdate(float aDeltaTime)
        {
            DoBlinkEffectPPFX();
            DoTakeDamagePPFX();
        }

        public Entity SpawnPrefabVFX(Prefab vfx, float lifeTime)
        {
            if (vfx == null)
            {
                Log.Error("Null VFX Prefab");
                return null;
            }

            Entity newVfx = Entity.Create(vfx);
            entity.CreateTimer(lifeTime, () => { Entity.Destroy(newVfx); });

            return newVfx;
        }

        public void ImpactVFX(ImapctType impactType, Vector3 pos, Quaternion rot)
        {
            Entity impactVfx = null;

            switch (impactType)
            {
                case ImapctType.Wall:
                    impactVfx = Entity.Create(WallImpact);
                    break;
                case ImapctType.Body:
                    impactVfx = Entity.Create(BodyImpact);
                    break;
                default:
                    break;
            }

            if (impactVfx == null)
            {
                Log.Error("Null Impact VFX Prefab");
                return;
            }

            impactVfx.position = pos;
            impactVfx.rotation = rot;

            entity.CreateTimer(2f, () => { Entity.Destroy(impactVfx); });
        }

        public void StartBlinkEffectPPFX()
        {
            if (BlinkEffect == null) return;

            blinkEffectActive = true;
            blinkCurrentTime = 0f;
            currentBlinkValue = 0f;

            BlinkEffect.SetFloat("blinkValue", 0f);
        }

        public void SetTakeDamagePPFX(float aMaxHealth, float aCurrent)
        {
            float effectVal = (aCurrent / aMaxHealth) * 4f;
            currentDamageValue = 4f - effectVal;
            TakeDamageEffect.SetFloat("lerpVal", currentDamageValue);
            timeUntilRegen = 3f;
        }

        private void DoTakeDamagePPFX()
        {
            if (currentDamageValue > 0)
            {
                if (timeUntilRegen <= 0)
                {
                    currentDamageValue -= disappearanceSpeed * Time.deltaTime;
                    TakeDamageEffect.SetFloat("lerpVal", currentDamageValue);
                }
                else
                {
                    timeUntilRegen -= Time.deltaTime;
                }
            }
        }

        private void DoBlinkEffectPPFX()
        {
            if (!blinkEffectActive) { return; }

            if (blinkCurrentTime < blinkDuration)
            {
                currentBlinkValue = Mathf.BounceIn(0f, 0.8f, blinkCurrentTime, blinkDuration);
                Log.Info(currentBlinkValue.ToString());
                BlinkEffect.SetFloat("blinkValue", currentBlinkValue);
                blinkCurrentTime += Time.deltaTime;
            }
            else
            {
                blinkEffectActive = false;
            }
        }
    }
}
