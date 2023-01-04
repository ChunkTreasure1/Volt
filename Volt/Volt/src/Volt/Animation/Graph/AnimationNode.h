#pragma once

#include "Volt/Core/UUID.h"

#include <string>
#include <vector>
#include <any>

namespace Volt
{
	namespace AnimationGraph
	{
		enum class Type
		{
			Sample,
			Data
		};

		enum class Direction
		{
			Input,
			Output
		};

		struct Pin
		{
			std::string name;
			UUID id{};

			std::vector<UUID> links;

			std::any data;

			Type type = Type::Sample;
			Direction direction = Direction::Input;
		};

		class Node
		{
		public:

		private:
		};
	}
}