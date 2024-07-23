#pragma once

#include "Mosaic/FormatterExtension.h"

#include <CoreUtilities/Core.h>
#include <CoreUtilities/Containers/Graph.h>
#include <CoreUtilities/VoltGUID.h>

#include <cstdint>

namespace Mosaic
{
	class MosaicNode;
	class MosaicEdge;

	class MosaicGraph
	{
	public:
		MosaicGraph();
		~MosaicGraph();

		void AddNode(const UUID64 uuid, const VoltGUID guid);
		void AddNode(const VoltGUID guid);

		uint32_t GetNextVariableIndex();
		uint32_t GetNextTextureIndex();
		const std::string GetNextVariableName();

		void ForfeitTextureIndex(uint32_t textureIndex);

		inline const uint32_t GetTextureCount() { return m_textureCount; }

		inline std::string& GetEditorState() { return m_editorState; }
		inline const std::string& GetEditorState() const { return m_editorState; }

		inline Graph<Ref<MosaicNode>, Ref<MosaicEdge>>& GetUnderlyingGraph() { return m_graph; }
		inline const Graph<Ref<MosaicNode>, Ref<MosaicEdge>>& GetUnderlyingGraph() const { return m_graph; }

		const std::string Compile() const;

		static Scope<MosaicGraph> CreateDefaultGraph();

		Graph<Ref<MosaicNode>, Ref<MosaicEdge>> m_graph;

	private:
		uint32_t m_currentVariableCount = 0;
		std::string m_editorState;

		Vector<uint32_t> m_availiableTextureIndices;

		uint32_t m_textureCount = 0;
		uint32_t m_currentTextureIndex = 0;
	};
}
