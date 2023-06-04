using System.Reactive.Joins;
using System.Reflection;
using Volt;

namespace Project
{
    public class BrokenLamp : Script
    {
        public bool IsCrazy
        {
            get
            {
                return myIsCrazy;
            }

            set
            {
                myIsCrazy = value;
                myIndex = 0;
            }
        }
        private bool myIsCrazy = false;

        private float[] myPattern = { 5f, 0.1f, 0.1f, 0.1f, 4f, 0.2f, 0.1f, 0.1f, 2f, 0.8f, 0.1f, 0.2f, 3f, 1f, 0.1f, 0.3f }; // pattern of on/off times in seconds
        private float[] myCrazyPattern = { 0.1f, 0.1f };

        private int myIndex = 0;
        private float myTimer = 0f;

        private float myStartIntensity = 0f;
        private float myColorSpeed = 5f;
        private float myElapsedTime = 0f;

        private void OnCreate()
        {
            if (entity != null)
            {
                if (entity.HasComponent<PointLightComponent>())
                {
                    myStartIntensity = entity.GetComponent<PointLightComponent>().intensity;
                }

                if (entity.HasComponent<SpotLightComponent>())
                {
                    myStartIntensity = entity.GetComponent<SpotLightComponent>().intensity;
                }
            }

            myStartIntensity = 2f;
        }

        private void OnUpdate(float deltaTime)
        {
            if (entity == null) return;

            myTimer += Time.deltaTime;
            myElapsedTime += Time.deltaTime;

            if (IsCrazy)
            {
                if (entity.HasComponent<SpotLightComponent>())
                {
                    SpotLightComponent spotlight = entity.GetComponent<SpotLightComponent>();
                    Vector3 rainbowColor = spotlight.color;
                    rainbowColor.x = Mathf.Sin(myElapsedTime * myColorSpeed) * 0.5f + 0.5f;
                    rainbowColor.y = Mathf.Sin((myElapsedTime + 1f) * myColorSpeed) * 0.5f + 0.5f;
                    rainbowColor.z = Mathf.Sin((myElapsedTime + 2f) * myColorSpeed) * 0.5f + 0.5f;
                    spotlight.color = rainbowColor;
                }
            }

            if (myTimer >= ((IsCrazy) ? myCrazyPattern[myIndex] : myPattern[myIndex]))
            {
                if (myIndex % 2 == 0)
                {
                    if (entity.HasComponent<PointLightComponent>())
                    {
                        entity.GetComponent<PointLightComponent>().intensity = myStartIntensity;
                    }
                    else if (entity.HasComponent<SpotLightComponent>())
                    {
                        entity.GetComponent<SpotLightComponent>().intensity = (IsCrazy) ? myStartIntensity * 2 : myStartIntensity;
                    }
                }
                else
                {
                    if (entity.HasComponent<PointLightComponent>())
                    {
                        entity.GetComponent<PointLightComponent>().intensity = 0;
                    }
                    else if (entity.HasComponent<SpotLightComponent>())
                    {
                        entity.GetComponent<SpotLightComponent>().intensity = 0;
                    }
                }

                myIndex++;

                if (myIndex >= ((IsCrazy) ? myCrazyPattern.Length : myPattern.Length))
                {
                    myIndex = 0;
                }

                myTimer = 0f;
            }
        }
    }
}
