#pragma once

#include <string>
#include <vector>

namespace Volt
{
	struct PhysicsLayer
	{
		uint32_t layerId = 0;
		uint32_t bitValue = 0;
		int32_t collidesWith = 0;
		std::string name;

		const bool IsValid() const
		{
			return !name.empty() && bitValue > 0;
		}
	};

	class PhysicsLayerManager
	{
	public:
		static uint32_t AddLayer(const std::string& name, bool setCollisions = true);
		static void AddLayer(PhysicsLayer layer);
		static void RemoveLayer(uint32_t layerId);

		static void SetLayerCollision(uint32_t layer, uint32_t otherLayer, bool shouldCollide);
		static bool ShouldCollide(uint32_t layer, uint32_t otherLayer);
		static bool IsValid(uint32_t layerId);

		inline static std::vector<PhysicsLayer>& GetLayers() { return myLayers; }

		static PhysicsLayer& GetLayer(uint32_t layerId);
		static PhysicsLayer& GetLayer(const std::string& layerName);

		static void Clear();

	private:
		PhysicsLayerManager() = delete;
	
		static uint32_t GetNextLayerId();

		inline static std::vector<PhysicsLayer> myLayers;
		inline static PhysicsLayer myNullLayer;
	};
}