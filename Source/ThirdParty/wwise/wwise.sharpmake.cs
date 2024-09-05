using Sharpmake;
using System;
using System.IO;

namespace Volt
{
    [Sharpmake.Export]
    public class wwise : Sharpmake.Project
    {
        public wwise() : base(typeof(CommonTarget))
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "wwise";
        }

        [Configure()]
        public void Configure(Configuration conf, CommonTarget target)
        {
            conf.IncludePaths.Add(@"[project.RootPath]\include");
            conf.LibraryPaths.Add(@"[project.RootPath]\lib\" + target.Optimization.ToString());

            conf.LibraryFiles.Add(
                "McDSPLimiterFX.lib",
                "McDSPFutzBoxFX.lib",
                "iZTrashMultibandDistortionFX.lib",
                "iZTrashFiltersFX.lib",
                "iZTrashDynamicsFX.lib",
                "iZTrashDistortionFX.lib",
                "iZTrashDelayFX.lib",
                "iZTrashBoxModelerFX.lib",
                "iZHybridReverbFX.lib",
                "CommunicationCentral.lib",
                "AuroHeadphoneFX.lib",
                "AkVorbisDecoder.lib",
                "AkTremoloFX.lib",
                "AkToneSource.lib",
                "AkTimeStretchFX.lib",
                "AkSynthOneSource.lib",
                "AkStreamMgr.lib",
                "AkStereoDelayFX.lib",
                "AkSpatialAudio.lib",
                "AkSoundSeedWooshSource.lib",
                "AkSoundSeedWindSource.lib",
                "AkSoundEngine.lib",
                "AkSineSource.lib",
                "AkSilenceSource.lib",
                "AkRoomVerbFX.lib",
                "AkReflectFX.lib",
                "AkRecorderFX.lib",
                "AkPitchShifterFX.lib",
                "AkPeakLimiterFX.lib",
                "AkParametricEQFX.lib",
                "AkOpusDecoder.lib",
                "AkMusicEngine.lib",
                "AkMeterFX.lib",
                "AkMemoryMgr.lib",
                "AkMatrixReverbFX.lib",
                "AkHarmonizerFX.lib",
                "AkGuitarDistortionFX.lib",
                "AkGainFX.lib",
                "AkFlangerFX.lib",
                "AkExpanderFX.lib",
                "AkDelayFX.lib",
                "AkCompressorFX.lib",
                "AkAutobahn.lib",
                "AkAudioInputSource.lib",
                "Ak3DAudioBedMixerFX.lib"
                );

            conf.Output = Configuration.OutputType.None;
        }
    }
}
