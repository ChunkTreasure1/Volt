#include "vtpch.h"
#include "Video.h"

#include "Volt/Log/Log.h"
#include "Volt/Rendering/Texture/Texture2D.h"

#include <VoltRHI/Images/Image2D.h>

namespace Volt
{
	namespace Utility
	{
		double R2D(AVRational r)
		{
			return r.num == 0 || r.den == 0 ? 0. : (double)r.num / (double)r.den;
		}

#define CALC_FFMPEG_VERSION(a,b,c) ( a<<16 | b<<8 | c )
		double GetFPS(AVFormatContext* aContext, int aStream)
		{
			double eps_zero = 0.000025;
			double fps = R2D(aContext->streams[aStream]->r_frame_rate);

#if LIBAVFORMAT_BUILD >= CALC_FFMPEG_VERSION(52, 111, 0)
			if (fps < eps_zero)
			{
				fps = R2D(aContext->streams[aStream]->avg_frame_rate);
			}
#endif

			if (fps < eps_zero)
			{
				fps = 1.0 / R2D(aContext->streams[aStream]->codec->time_base);
			}

			return fps;
		}
	}

	Video::~Video()
	{
		Release();
	}

	void Video::Initialize(const std::filesystem::path& filePath)
	{
		av_register_all();

		int32_t result = avformat_open_input(&myReaderState.formatContext, filePath.string().c_str(), nullptr, nullptr);
		if (result < 0)
		{
			VT_CORE_ERROR("Error when opening video!");
			return;
		}

		result = avformat_find_stream_info(myReaderState.formatContext, nullptr);

		if (result >= 0)
		{
			for (uint32_t i = 0; i < myReaderState.formatContext->nb_streams; i++)
			{
				auto& stream = myReaderState.formatContext->streams[i];

				if (stream->codec->codec_type == AVMEDIA_TYPE_VIDEO)
				{
					myReaderState.videoStreamIndex = i;
					myReaderState.decoderContext = stream->codec;
					myReaderState.codec = avcodec_find_decoder(stream->codec->codec_id);
				}
				else if (stream->codec->codec_type == AVMEDIA_TYPE_AUDIO)
				{
					// TODO: implement audio
				}
			}
		}

		if (myReaderState.codec && myReaderState.decoderContext)
		{
			result = avcodec_open2(myReaderState.decoderContext, myReaderState.codec, nullptr);

			if (result >= 0)
			{
				myReaderState.avFrame = avcodec_alloc_frame();
				myReaderState.avFrameBGR = avcodec_alloc_frame();

				AVPixelFormat format = AV_PIX_FMT_RGBA;

				myNumberOfBytes = avpicture_get_size(format, myReaderState.decoderContext->width, myReaderState.decoderContext->height);
				myBuffer = (uint8_t*)av_malloc(myNumberOfBytes);

				avpicture_fill((AVPicture*)myReaderState.avFrameBGR, myBuffer, format, myReaderState.decoderContext->width, myReaderState.decoderContext->height);

				auto decodeContext = myReaderState.decoderContext;
				myReaderState.swsContext = sws_getContext(decodeContext->width, decodeContext->height, decodeContext->pix_fmt, decodeContext->width, decodeContext->height, format, SWS_BILINEAR, nullptr, nullptr, nullptr);
			}
		}

		ReadNextFrame();

		myReaderState.width = myReaderState.avFrame->width;
		myReaderState.height = myReaderState.avFrame->height;

		// Image
		{
			RHI::ImageSpecification spec{};
			spec.format = RHI::PixelFormat::R8G8B8A8_UNORM;
			spec.usage = RHI::ImageUsage::Storage;
			spec.width = myReaderState.width;
			spec.height = myReaderState.height;
			spec.isCubeMap = false;
			spec.layers = 1;
			spec.debugName = "Video Image";

			myImage = RHI::Image2D::Create(spec);
		}


		myIsPlaying = true;
		myCurrentFrameTime = 0.0001f;
		Update(0.f);
		myCurrentFrameTime = 0.f;
		myIsPlaying = false;

		myStatus = VideoStatus::Playing;
	}

	void Video::Play(bool shouldLoop)
	{
		myIsPlaying = true;
		myIsLooping = shouldLoop;
		myStatus = VideoStatus::Playing;
	}

	void Video::Pause()
	{
		myIsPlaying = false;
	}

	void Video::Stop()
	{
		myIsPlaying = false;
		myStatus = VideoStatus::Stopped;
		av_seek_frame(myReaderState.formatContext, myReaderState.videoStreamIndex, 0, AVSEEK_FLAG_ANY);
	}

	void Video::Update(float deltaTime)
	{
		if (!myIsPlaying)
		{
			return;
		}

		myCurrentFrameTime += deltaTime;

		float fps = (float)Utility::GetFPS(myReaderState.formatContext, myReaderState.videoStreamIndex);

		while (myCurrentFrameTime >= 1.f / fps)
		{
			int32_t result = ReadNextFrame();
			if (result < 0)
			{
				myStatus = VideoStatus::End;
				if (myIsLooping)
				{
					Restart();
				}
			}

			//uint32_t* data = myImage->Map<uint32_t>();
			//GetFrameData(data);
			//myImage->Unmap();

			myCurrentFrameTime -= 1.f / fps;
		}
	}

	int32_t Video::ReadNextFrame()
	{
		bool valid = false;
		int32_t readFrame = 0;
		int32_t safeValue = 0;

		const int32_t maxSafeValue = 10000;

		while (!valid || safeValue > maxSafeValue)
		{
			safeValue++;
			readFrame = av_read_frame(myReaderState.formatContext, &myReaderState.avPacket);
			if (readFrame >= 0)
			{
				if (myReaderState.avPacket.stream_index == myReaderState.videoStreamIndex)
				{
					avcodec_decode_video2(myReaderState.decoderContext, myReaderState.avFrame, &myGotFrame, &myReaderState.avPacket);

					if (myGotFrame)
					{
						valid = true;
					}
				}
			}
			else
			{
				valid = true;
			}
			av_free_packet(&myReaderState.avPacket);
		}

		if (safeValue >= maxSafeValue)
		{
			VT_CORE_ERROR("Unable to find a valid video frame!");
		}

		return readFrame;
	}

	void Video::Release()
	{
		if (myReaderState.avFrame)
		{
			av_free(myReaderState.avFrame);
		}

		if (myReaderState.avFrameBGR)
		{
			av_free(myReaderState.avFrameBGR);
		}

		if (myReaderState.decoderContext)
		{
			avcodec_close(myReaderState.decoderContext);
		}

		if (myReaderState.formatContext)
		{
			avformat_close_input(&myReaderState.formatContext);
		}

		if (myBuffer)
		{
			av_free(myBuffer);
		}

		if (myReaderState.swsContext)
		{
			sws_freeContext(myReaderState.swsContext);
		}
	}

	void Video::Restart()
	{
		av_seek_frame(myReaderState.formatContext, myReaderState.videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD);
		myIsPlaying = true;
	}

	bool Video::GetFrameData(uint32_t*& buffer)
	{
		if (!myGotFrame)
		{
			return false;
		}

		int32_t result = sws_scale(myReaderState.swsContext, myReaderState.avFrame->data, myReaderState.avFrame->linesize, 0, myReaderState.decoderContext->height,
			myReaderState.avFrameBGR->data, myReaderState.avFrameBGR->linesize);

		if (result > 0)
		{
			int32_t rgbIndex = 0;
			for (int32_t i = 0; i < myReaderState.decoderContext->height; i++)
			{
				for (int32_t j = 0; j < myReaderState.decoderContext->width; j++)
				{
					uint8_t* data = myReaderState.avFrameBGR->data[0];
					buffer[i * myReaderState.width + j] = int32_t(data[rgbIndex + 3]) << 24 |
						(data[rgbIndex + 2]) << 16 |
						(data[rgbIndex + 1]) << 8 |
						(data[rgbIndex]);

					rgbIndex += 4;
				}
			}
		}

		return true;
	}
}
