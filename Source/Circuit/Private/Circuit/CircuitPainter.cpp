#include "circuitpch.h"
#include "CircuitPainter.h"

#include <Volt-Assets/Assets/Font.h>
#include <Volt-Assets/Assets/MSDFData.h>

#include <CoreUtilities/StringUtility.h>

namespace Circuit
{
	CircuitPainter::CircuitPainter()
	{
	}

	CircuitPainter::CircuitPainter(const Volt::Rect& allotedArea)
		:m_allottedArea(allotedArea)
	{
	}

	CircuitPainter::CircuitPainter(const glm::vec2& position, const glm::vec2& size)
		:m_allottedArea(position, size)
	{
	}

	CircuitPainter::CircuitPainter(CircuitPainter* basePainter, const Volt::Rect& allotedArea)
		:m_allottedArea(allotedArea),
		m_basePainter(basePainter)
	{
	}

	CircuitPainter::CircuitPainter(CircuitPainter* basePainter, const glm::vec2& position, const glm::vec2& size)
		:m_allottedArea(position, size),
		m_basePainter(basePainter)
	{
	}

	const Volt::Rect& CircuitPainter::GetAllotedArea() const
	{
		return m_allottedArea;
	}

	CircuitPainter CircuitPainter::CreateSubPainter(const Volt::Rect& allotedArea)
	{
		return CircuitPainter(m_basePainter ? m_basePainter : this, allotedArea);
	}

	CircuitPainter CircuitPainter::CreateSubPainter(const glm::vec2& position, const glm::vec2& size)
	{
		return CircuitPainter(m_basePainter ? m_basePainter : this, position, size);
	}

	void CircuitPainter::AddRect(float x, float y, float width, float height, CircuitColor color, float rotation, float scale)
	{
		CircuitDrawCommand command;
		command.type = CircuitPrimitiveType::Rect;

		command.pixelPos.x = x;
		command.pixelPos.y = y;

		command.radiusHalfSize.x = width / 2;
		command.radiusHalfSize.y = height / 2;

		command.rotation = rotation;

		command.scale = scale;

		command.color = color;

		std::vector<CircuitDrawCommand>* drawCommandsToAppendTo = &m_drawCommands;
		if (m_basePainter)
		{
			drawCommandsToAppendTo = &m_basePainter->m_drawCommands;
		}
		drawCommandsToAppendTo->push_back(command);
	}

	void CircuitPainter::AddCircle(float x, float y, float radius, CircuitColor color, float scale)
	{
		CircuitDrawCommand command;
		command.type = CircuitPrimitiveType::Circle;
		command.pixelPos.x = x;
		command.pixelPos.y = y;

		command.radiusHalfSize.x = radius;

		command.scale = scale;

		command.color = color;


		std::vector<CircuitDrawCommand>* drawCommandsToAppendTo = &m_drawCommands;
		if (m_basePainter)
		{
			drawCommandsToAppendTo = &m_basePainter->m_drawCommands;
		}
		drawCommandsToAppendTo->push_back(command);
	}

	void CircuitPainter::AddText(float x, float y, const std::string& text, Ref<Volt::Font> font, float maxWidth, CircuitColor color, float scale)
	{
		if (text.empty())
		{
			return;
		}
		
		std::u32string utf32string = ::Utility::ToUTF32(text);

		auto& fontGeom = font->GetMSDFData()->fontGeometry;
		const auto& metrics = fontGeom.getMetrics();

		Vector<int32_t> lineSplits;

		// Find all line splits
		{
			double sX = 0.0;
			double fsScale = 1.0 / (metrics.ascenderY - metrics.descenderY);
			double sY = -fsScale * metrics.ascenderY;

			int32_t lastSpace = -1;

			for (int32_t i = 0; i < static_cast<int32_t>(utf32string.size()); i++)
			{
				char32_t character = utf32string[i];
				if (character == '\n')
				{
					sX = 0.0;
					sY -= fsScale * metrics.lineHeight;
					continue;
				}

				auto glyph = fontGeom.getGlyph(character);
				if (!glyph)
				{
					glyph = fontGeom.getGlyph('?');
				}

				VT_ENSURE(glyph);

				if (character != ' ')
				{
					double pl, pb, pr, pt;
					glyph->getQuadPlaneBounds(pl, pb, pr, pt);
					glm::vec2 quadMin((float)pl, (float)pb);
					glm::vec2 quadMax((float)pl, (float)pb);

					quadMin *= (float)fsScale;
					quadMax *= (float)fsScale;
					quadMin += glm::vec2((float)sX, (float)sY);
					quadMax += glm::vec2((float)sX, (float)sY);

					if (quadMax.x > maxWidth && lastSpace != -1)
					{
						i = lastSpace;
						lineSplits.emplace_back(lastSpace);
						lastSpace = -1;
						sX = 0.0;
						sY -= fsScale * metrics.lineHeight;
					}
				}
				else
				{
					lastSpace = i;
				}

				double advance = glyph->getAdvance();
				fontGeom.getAdvance(advance, character, utf32string[i + 1]);
				sX += fsScale * advance;
			}
		}

		// Setup commands
		{
			double sX = 0.0;
			double fsScale = 1.0 / (metrics.ascenderY - metrics.descenderY);
			double sY = -fsScale * metrics.ascenderY;

			for (int32_t i = 0; i < static_cast<int32_t>(utf32string.size()); i++)
			{
				char32_t character = utf32string[i];
				if (character == '\n')
				{
					sX = 0.0;
					sY += fsScale * metrics.lineHeight;
					continue;
				}

				auto glyph = fontGeom.getGlyph(character);
				if (!glyph)
				{
					glyph = fontGeom.getGlyph('?');
				}

				VT_ENSURE(glyph);

				double l, b, r, t;
				glyph->getQuadAtlasBounds(l, b, r, t);

				double pl, pb, pr, pt;
				glyph->getQuadPlaneBounds(pl, pb, pr, pt);

				pt = metrics.ascenderY - pt;
				pb = metrics.ascenderY - pb;

				pt += metrics.ascenderY + metrics.descenderY;
				pb += metrics.ascenderY + metrics.descenderY;

				pl *= fsScale, pb *= fsScale, pr *= fsScale, pt *= fsScale;
				pl += sX, pb += sY, pr += sX, pt += sY;

				double texelWidth = 1.0 / font->GetAtlas()->GetWidth();
				double texelHeight = 1.0 / font->GetAtlas()->GetHeight();

				l *= texelWidth, b *= texelHeight, r *= texelWidth, t *= texelHeight;

				std::vector<CircuitDrawCommand>* drawCommandsToAppendTo = &m_drawCommands;
				if (m_basePainter)
				{
					drawCommandsToAppendTo = &m_basePainter->m_drawCommands;
				}

				CircuitDrawCommand& cmd = drawCommandsToAppendTo->emplace_back();
				cmd.type = CircuitPrimitiveType::TextCharacter;
				cmd.pixelPos.x = x;
				cmd.pixelPos.y = y;
				cmd.color = color;
				cmd.scale = scale;

				cmd.minMaxPx.x = static_cast<float>(pl) * scale + x;
				cmd.minMaxPx.y = static_cast<float>(pb) * scale + y;
				cmd.minMaxPx.z = static_cast<float>(pr) * scale + x;
				cmd.minMaxPx.w = static_cast<float>(pt) * scale + y;

				cmd.minMaxUV.x = static_cast<float>(l);
				cmd.minMaxUV.y = static_cast<float>(b);
				cmd.minMaxUV.z = static_cast<float>(r);
				cmd.minMaxUV.w = static_cast<float>(t);

				cmd.texture = font->GetAtlasResourceHandle();

				double advance = glyph->getAdvance();
				fontGeom.getAdvance(advance, character, utf32string[i + 1]);
				sX += fsScale * advance;
			}
		}
	}

	std::vector<CircuitDrawCommand> CircuitPainter::GetCommands()
	{
		return m_drawCommands;
	}

}
