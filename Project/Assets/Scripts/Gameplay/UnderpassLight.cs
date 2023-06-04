using System.Collections.Generic;
using System.Linq;
using System.Xml.Serialization;
using Volt;

namespace Project
{
    public class UnderpassLight : Script
    {
        Entity[] lights;
        List<Material> myMaterial;

        private void OnCreate()
        {
            myMaterial = new List<Material>();
            lights = entity.children;
            foreach(Entity ent in lights)
            {
                if(ent.HasComponent<SpotLightComponent>() || ent.HasComponent<PointLightComponent>())
                {
                    continue;
                }
                Material mat = ent.GetComponent<MeshComponent>()?.material.CreateCopy();
                mat.emissiveStrength = 0;
                ent.GetComponent<MeshComponent>().material = mat;
                myMaterial.Add(ent.GetComponent<MeshComponent>().material);
            }
        }
        public void TurnOnLights()
        {
            Log.Info("oy");
            for(int i = 0; i < lights.Length; i++)
            {
                if (lights[i].HasComponent<SpotLightComponent>())
                {
                    lights[i].GetComponent<SpotLightComponent>().intensity = 0.49f;
                }
                if (lights[i].HasComponent<PointLightComponent>())
                {
                    lights[i].GetComponent<PointLightComponent>().intensity = 0.48f;
                }
            }
            for (int i = 0; i < myMaterial.Count; i++)
            {
                myMaterial[i].emissiveStrength = 2000;
            }
        }
    }
}
