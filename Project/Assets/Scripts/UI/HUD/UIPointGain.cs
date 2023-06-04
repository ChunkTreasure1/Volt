using System;
using Volt;

namespace Project
{
    public class UIPointGain : Script
    {
        public float Speed;

        private void OnCreate()
        {
            entity.CreateTimer(1.2f, () => { Entity.Destroy(entity); });
        }

        private void OnUpdate(float deltaTime)
        {
            entity.position += new Vector3(0, Speed * deltaTime, 0);
            entity.GetScript<Text>().TextColor.w = Mathf.Lerp(entity.GetScript<Text>().TextColor.w, 0, 1 - (float)Math.Pow(2f, -4 * Time.deltaTime));
        }
    }
}
