#pragma once

#include <Volt/Core/Base.h>
#include <Volt/Rendering/Framebuffer.h>
#include <NodeEditor/Node.h>

class FramebufferNode : public NE::Node
{
public:
	FramebufferNode();

	void OnCreate() override;
	void DrawContent() override;

private:
	inline static const std::vector<const char*> myImageFormatStrings =
	{
		"None",
		"R32F",
		"R16F",
		"R32SI",
		"R32UI",
		"R8U",
		"RGBA",
		"RGBAS",
		"RGBA16F",
		"RGBA32F",
		"RG16F",
		"RG32F",

		"BC1",
		"BC1SRGB",
		"BC2",
		"BC2SRGB",
		"BC3",
		"BC3SRGB",
		"BC4",
		"BC5",
		"BC7",
		"BC7SRGB",

		"R32Typeless",
		"R16Typeless",

		"DEPTH32F",
		"DEPTH16U",
		"DEPTH24STENCIL8"
	};

	void Rebuild();
	
	Volt::FramebufferSpecification mySpecification;
	int32_t myCurrentImageFormat = 0;
};