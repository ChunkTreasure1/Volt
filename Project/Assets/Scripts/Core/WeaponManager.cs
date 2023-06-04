using System.Collections.Generic;
using System.Diagnostics;
using Volt;

namespace Project
{
    public enum WeaponId
    {
        Unknown,
        Galil,
        Kar98k,
        M1911,
        Spas12,
        Thunder,
        COUNT,
    }
    public struct WeaponMeta
    {
        public AssetHandle StaticMesh;
    }

    public class DebugWeaponTrail
    {
        public Vector3 start;
        public Vector3 end;
        public bool hit;
        public float lifetime;
        
        public bool Update()
        {
            lifetime -= Time.deltaTime;

            DebugRenderer.DrawLine(start, end, (hit) ? new Vector4(1, 0, 0, 1) : new Vector4(0, 1, 0, 1));
            
            if (lifetime <= 0.0f)
            {
                return true;
            }
            return false;
        }
    }

    public class WeaponManager : Script
    {
        #region Static Instance
        static WeaponManager _instance;
        public static WeaponManager Instance
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
        #endregion

        WeaponMeta[] metaData = new WeaponMeta[(int)WeaponId.COUNT];
        List<DebugWeaponTrail> debugWeaponTrails = new List<DebugWeaponTrail>();

        private void OnAwake()
        {
            _instance = this;
            SetupMetadata();
        }

        private void OnUpdate(float deltaTime)
        {
            for (int i = debugWeaponTrails.Count - 1; i >= 0; i--)
            {
                if (debugWeaponTrails[i].Update())
                {
                    debugWeaponTrails.RemoveAt(i);
                }
            }
        }

        public void AddDebugWeaponTrail(DebugWeaponTrail trail)
        {
            debugWeaponTrails.Add(trail);
        }

        public WeaponMeta GetMetadata(WeaponId id) 
        {
            return metaData[(int)id];
        }

        public WeaponBehaviour CreateWeapon(WeaponId id) 
        {
            switch (id)
            {
                case WeaponId.Galil:
                    {
                        return new Galil();
                    }
                case WeaponId.Kar98k:
                    {
                        return new Kar98k();
                    }
                case WeaponId.M1911:
                    {
                        return new M1911();
                    }
                case WeaponId.Spas12:
                    {
                        return new Spas12();
                    }
                case WeaponId.Thunder:
                    {
                        return new Thunder();
                    }
                default:
                    {
                        return new WeaponBehaviour();
                    }
            }
        }

        private void SetupMetadata()
        {
            for (int i = 0; i < (int)WeaponId.COUNT; ++i)
            {
                WeaponMeta weaponMeta = new WeaponMeta();

                switch ((WeaponId)i)
                {
                    case WeaponId.Galil:
                        {
                            weaponMeta.StaticMesh = AssetManager.GetAssetHandleFromPath("Assets/Characters/Player/HandGun/handgun_SK.vtmesh");
                            break;
                        }
                    case WeaponId.Kar98k:
                        {
                            weaponMeta.StaticMesh = AssetManager.GetAssetHandleFromPath("Assets/Characters/Player/HandGun/handgun_SK.vtmesh");
                            break;
                        }
                    case WeaponId.M1911:
                        {
                            weaponMeta.StaticMesh = AssetManager.GetAssetHandleFromPath("Assets/Characters/Player/HandGun/handgun_SK.vtmesh");
                            break;
                        }
                    case WeaponId.Spas12:
                        {
                            weaponMeta.StaticMesh = AssetManager.GetAssetHandleFromPath("Assets/Characters/Player/HandGun/handgun_SK.vtmesh");
                            break;
                        }
                    case WeaponId.Thunder:
                        {
                            weaponMeta.StaticMesh = AssetManager.GetAssetHandleFromPath("Assets/Characters/Player/HandGun/handgun_SK.vtmesh");
                            break;
                        }
                    default:
                        {
                            weaponMeta.StaticMesh = 0;
                            break;
                        }
                }

                metaData[i] = weaponMeta;
            }
        }
    }
}
