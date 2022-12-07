#include "vtpch.h"
#include "PhysicsLayer.h"

#include "Volt/Core/Base.h"
#include "Volt/Log/Log.h"

namespace Volt
{
	uint32_t PhysicsLayerManager::AddLayer(const std::string& name, bool setCollisions)
	{
		const uint32_t layerId = GetNextLayerId();
		PhysicsLayer layer = { layerId, (uint32_t)BIT(layerId), (int32_t)BIT(layerId), name };
		myLayers.insert(myLayers.begin() + layerId, layer);

		if (setCollisions)
		{
			for (const auto& otherLayer : myLayers)
			{
				SetLayerCollision(layerId, otherLayer.layerId, true);
			}
		}

		return layerId;
	}

	void PhysicsLayerManager::AddLayer(PhysicsLayer layer)
	{
		myLayers.insert(myLayers.begin() + layer.layerId, layer);

		for (const auto& otherLayer : myLayers)
		{
			if (layer.collidesWith & otherLayer.bitValue)
			{
				SetLayerCollision(otherLayer.layerId, layer.layerId, true);
			}
		}
	}

	void PhysicsLayerManager::RemoveLayer(uint32_t layerId)
	{
		PhysicsLayer& layerInfo = GetLayer(layerId);
		if (layerInfo.layerId == myNullLayer.layerId)
		{
			VT_CORE_ERROR("Trying to remove invalid physics layer!");
			return;
		}

		for (auto& otherLayer : myLayers)
		{
			if (otherLayer.layerId == layerId)
			{
				continue;
			}

			if (otherLayer.collidesWith & layerInfo.bitValue)
			{
				otherLayer.collidesWith &= ~layerInfo.bitValue;
			}
		}

		auto it = std::find_if(myLayers.begin(), myLayers.end(), [layerId](const PhysicsLayer& physLayer)
			{
				return physLayer.layerId == layerId;
			});

		if (it != myLayers.end())
		{
			myLayers.erase(it);
		}
	}

	void PhysicsLayerManager::SetLayerCollision(uint32_t layer, uint32_t otherLayer, bool shouldCollide)
	{
		if (ShouldCollide(layer, otherLayer) && shouldCollide)
		{
			return;
		}

		auto& layerInfo = GetLayer(layer);
		auto& otherLayerInfo = GetLayer(otherLayer);

		if (shouldCollide)
		{
			layerInfo.collidesWith |= otherLayerInfo.bitValue;
			otherLayerInfo.collidesWith |= layerInfo.bitValue;
		}
		else
		{
			layerInfo.collidesWith &= ~otherLayerInfo.bitValue;
			otherLayerInfo.collidesWith &= ~layerInfo.bitValue;
		}
	}

	bool PhysicsLayerManager::ShouldCollide(uint32_t layer, uint32_t otherLayer)
	{
		return GetLayer(layer).collidesWith & GetLayer(otherLayer).bitValue;
	}

	bool PhysicsLayerManager::IsValid(uint32_t layerId)
	{
		const auto& layer = GetLayer(layerId);
		return layer.layerId != myNullLayer.layerId && layer.IsValid();
	}

	PhysicsLayer& PhysicsLayerManager::GetLayer(uint32_t layerId)
	{
		for (auto& layer : myLayers)
		{
			if (layer.layerId == layerId)
			{
				return layer;
			}
		}

		return myNullLayer;
	}

	PhysicsLayer& PhysicsLayerManager::GetLayer(const std::string& layerName)
	{
		for (auto& layer : myLayers)
		{
			if (layer.name == layerName)
			{
				return layer;
			}
		}

		return myNullLayer;
	}

	void PhysicsLayerManager::Clear()
	{
		myLayers.clear();
	}

	uint32_t PhysicsLayerManager::GetNextLayerId()
	{
		int32_t lastId = -1;

		for (const auto& layer : myLayers)
		{
			if (lastId != -1)
			{
				if ((int32_t)layer.layerId != lastId + 1)
				{
					return lastId + 1;
				}
			}

			lastId = layer.layerId;
		}

		return (uint32_t)myLayers.size();
	}
}