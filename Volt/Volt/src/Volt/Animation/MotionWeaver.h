#pragma once
#include "Volt/Core/Base.h"
#include "glm/mat4x4.hpp"
#include "Volt/Asset/Asset.h"

namespace Volt
{
	class Animation;
	class MotionWeaveDatabase;
	class Skeleton;
	class MotionWeaver
	{
	public:
		MotionWeaver(Ref<MotionWeaveDatabase> database);
		~MotionWeaver() = default;

		static Ref<MotionWeaver> Create(AssetHandle databaseHandle);

		void SetDatabase(Ref<MotionWeaveDatabase> database);

		void Update(float deltaTime);

		std::vector<glm::mat4x4> Sample();


	private:
		Ref<MotionWeaveDatabase> m_Database;
		std::vector<Ref<Animation>> m_Animations;
		Ref<Skeleton> m_Skeleton;
		
		float m_Time;

	};

}
