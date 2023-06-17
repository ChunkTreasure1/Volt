#pragma once

#include "GraphKey/Node.h"

namespace GraphKey
{
	struct MaterialBaseNode : public Node
	{
		~MaterialBaseNode() override = default;

		virtual const std::string Evaluate(uint32_t pin) const = 0;
		virtual const std::string CompileFunction() const = 0;
	};

	struct TextureSampleNode : public MaterialBaseNode
	{
		TextureSampleNode();
		~TextureSampleNode();

		const std::string Evaluate(uint32_t pin) const override;
		const std::string CompileFunction() const override;

		inline const std::string GetName() override { return "Texture Sample"; }
		inline const gem::vec4 GetColor() override { return { 0.3f, 1.f, 0.49f, 1.f }; }
	};

	struct MaterialOutputNode : public MaterialBaseNode
	{
		MaterialOutputNode();
		~MaterialOutputNode() override = default;
	
		const std::string Evaluate(uint32_t pin) const override;
		const std::string CompileFunction() const override;

		inline const std::string GetName() override { return "Material Output"; }
		inline const gem::vec4 GetColor() override { return { 0.3f, 1.f, 0.49f, 1.f }; }
	};
}
