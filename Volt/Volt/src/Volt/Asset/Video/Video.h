#pragma once

#include "Volt/Asset/Asset.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/avutil.h>
#include <libpostproc/postprocess.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#include <libswscale/swscale.h>
#include <wtypes.h>
}

namespace Volt
{
	class Image2D;
	enum class VideoStatus : uint32_t
	{
		Stopped,
		Playing,
		End
	};

	class Video : public Asset
	{
		struct ReaderState
		{
			AVFormatContext* formatContext = nullptr;
			AVCodecContext* decoderContext = nullptr;

			AVFrame* avFrame = nullptr;
			AVFrame* avFrameBGR = nullptr;

			AVPacket avPacket;
			SwsContext* swsContext = nullptr;

			const AVCodec* codec = nullptr;

			uint32_t width;
			uint32_t height;
			int32_t videoStreamIndex = -1;
			AVRational timeBase;
		};

	public:
		Video() = default;
		Video(const std::filesystem::path& aPath);
		~Video();

		void Play(bool aLoop = false);
		void Pause();
		void Stop();
		void Restart();

		void Update(float aDeltaTime);
		inline VideoStatus GetStatus() { return myStatus; }
		inline Ref<Image2D> GetImage() const { return myImage; }

		static AssetType GetStaticType() { return AssetType::Video; }
		AssetType GetType() override { return GetStaticType(); };


	private:
		int32_t ReadNextFrame();

		void Release();
		bool GetFrameData(uint32_t*& aBuffer);

		Ref<Image2D> myImage;
		VideoStatus myStatus = VideoStatus::Stopped;

		int32_t myNumberOfBytes = 0;
		uint8_t* myBuffer = nullptr;
		int32_t myGotFrame = false;

		float myCurrentFrameTime = 0.f;
		bool myIsPlaying = false;
		bool myIsLooping = false;

		ReaderState myReaderState;
	};
}
