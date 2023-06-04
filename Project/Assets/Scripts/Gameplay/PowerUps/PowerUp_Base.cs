using Volt;

namespace Project
{
    public abstract class PowerUp_Base : Script
    {
        protected PowerUps myType = PowerUps.None;

        private float myNoiseStep = 0f;
        private int myStaticSeed = 0;

        protected void Init()
        {
            myStaticSeed = Volt.Random.Range(199, 101249);
            entity.position += new Vector3(0, 100, 0);
        }

        private void OnUpdate(float deltaTime)
        {
            Noise.StaticSeed = myStaticSeed;
            Noise.Frequency = 0.1f;
            float x = Noise.Perlin(myNoiseStep, 0, 0);
            float y = Noise.Perlin(0, myNoiseStep, 0);
            float z = Noise.Perlin(0, 0, myNoiseStep);

            Vector3 noiseVal = new Vector3(x, y, z);

            Quaternion noiseRot = Quaternion.Euler(noiseVal.x, noiseVal.y * 5, noiseVal.z);

            entity.rotation = noiseRot;

            myNoiseStep += deltaTime;
        }
    }
}
