#include "vtpch.h"
#include "SkyAtmosphereTechnique.h"

#include <RenderCore/RenderGraph/RenderGraph.h>
#include <RenderCore/RenderGraph/RenderGraphBlackboard.h>
#include <RenderCore/RenderGraph/RenderGraphUtils.h>

namespace Volt
{
	struct TransmittanceLUTData
	{
		RenderGraphResourceHandle transmittanceLUT;
	};

	struct AtmosphereParameters
	{
		// Radius of the planet (center to ground)
		float BottomRadius;
		// Maximum considered atmosphere height (center to atmosphere top)
		float TopRadius;

		// Rayleigh scattering exponential distribution scale in the atmosphere
		float RayleighDensityExpScale;
		// Rayleigh scattering coefficients
		glm::vec3 RayleighScattering;

		// Mie scattering exponential distribution scale in the atmosphere
		float MieDensityExpScale;
		// Mie scattering coefficients
		glm::vec3 MieScattering;
		// Mie extinction coefficients
		glm::vec3 MieExtinction;
		// Mie absorption coefficients
		glm::vec3 MieAbsorption;
		// Mie phase function excentricity
		float MiePhaseG;

		// Another medium type in the atmosphere
		float AbsorptionDensity0LayerWidth;
		float AbsorptionDensity0ConstantTerm;
		float AbsorptionDensity0LinearTerm;
		float AbsorptionDensity1ConstantTerm;
		float AbsorptionDensity1LinearTerm;
		// This other medium only absorb light, e.g. useful to represent ozone in the earth atmosphere
		glm::vec3 absorptionExtinction;

		// The albedo of the ground.
		glm::vec3 GroundAlbedo;
	};

	SkyAtmosphereTechnique::SkyAtmosphereTechnique(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
		: m_renderGraph(renderGraph), m_blackboard(blackboard)
	{
	}

	SkyAtmosphereData SkyAtmosphereTechnique::Execute()
	{
		//{
		//	const auto desc = RGUtils::CreateBufferDesc<AtmosphereParameters>(1, RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::CPUToGPU, "Atmosphere Buffer");
		//	RenderGraphResourceHandle atmosphereUniformBuffer = m_renderGraph.CreateUniformBuffer(desc);

		//	AtmosphereParameters params{};
		//	params.absorptionExtinction = { 0.000650f, 0.001881f, 0.000085f };
		//	params.RayleighDensityExpScale = 0.f;
		//}

		return SkyAtmosphereData();
	}

	RenderGraphResourceHandle SkyAtmosphereTechnique::RenderTransmittanceLUT()
	{
		constexpr uint32_t TRANSMITTANCE_LUT_WIDTH = 256;
		constexpr uint32_t TRANSMITTANCE_LUT_HEIGHT = 64;

		m_blackboard.Add<TransmittanceLUTData>() = m_renderGraph.AddPass<TransmittanceLUTData>("Transmittance LUT",
		[&](RenderGraph::Builder& builder, TransmittanceLUTData& data)
		{
			auto desc = RGUtils::CreateImage2DDesc<RHI::PixelFormat::R16G16B16A16_SFLOAT>(TRANSMITTANCE_LUT_WIDTH, TRANSMITTANCE_LUT_HEIGHT, RHI::ImageUsage::Attachment, "Sky Transmittance LUT");
			data.transmittanceLUT = builder.CreateImage2D(desc);
		},
		[=](const TransmittanceLUTData& data, RenderContext& context)
		{

		});

		return RenderGraphResourceHandle();
	}

	RenderGraphResourceHandle SkyAtmosphereTechnique::SetupAtmosphereUniformBuffer()
	{
		return RenderGraphResourceHandle();
	}
}
