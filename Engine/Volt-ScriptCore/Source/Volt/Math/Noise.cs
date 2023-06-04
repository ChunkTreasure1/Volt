using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Volt
{
	public class Noise
	{
        //public float Frequency
        //{
        //	get => InternalCalls.Noise_GetFrequency();
        //	set => InternalCalls.Noise_SetFrequency(value);
        //}

        //public int Octaves
        //{
        //	get => InternalCalls.Noise_GetFractalOctaves();
        //	set => InternalCalls.Noise_SetFractalOctaves(value);
        //}

        //public float Lacunarity
        //{
        //	get => InternalCalls.Noise_GetFractalLacunarity();
        //	set => InternalCalls.Noise_SetFractalLacunarity(value);
        //}

        //public float Gain
        //{
        //	get => InternalCalls.Noise_GetFractalGain();
        //	set => InternalCalls.Noise_SetFractalGain(value);
        //}

        //public float Get(float x, float y)
        //{
        //	return InternalCalls.Noise_Get(x, y);
        //}

        // --- Cherno stuff above ---

        public static float Frequency
        {
            set => InternalCalls.Noise_SetFrequency(value);
        }

        public static int StaticSeed
		{
			set => InternalCalls.Noise_SetSeed(value);
		}

		public static float Perlin(float x, float y, float z)
		{
			return InternalCalls.Noise_Perlin(x, y, z);
		}
	}
}
