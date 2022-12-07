#pragma once
#include <Volt/Core/Base.h>
#include "GEM/gem.h"
#include <vector>
#include <Volt/Scene/Scene.h>
#include "Volt/Scene/Entity.h"
#include "Volt/Components/Components.h"
#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Mesh/Mesh.h"

class Spline
{
public:
	Spline(Ref<Volt::Scene>& aScene, int aAmoutOfCubes);
	~Spline() = default;
	
	int GetNumSegments() { return ((int32_t)myPointEntitys.size() - 4) / 3 + 1; }
	std::vector<gem::vec3> GetPointsInSegment(int index);

	void CreateNewBezierPoints();
	void UpdateSpline();
	void InitSpline();
	void AddSegment();

private:
	std::vector<Volt::Entity> myPointEntitys;
	std::vector<std::vector<Volt::Entity>> myBezierePoints;

	Volt::Entity myMainParentSpline;
	Volt::Entity myBezierPointParent;

	int myAmountOfCubes;

	Ref<Volt::Scene>& myCurrentScene;

	gem::vec3 Lerp(gem::vec3 a, gem::vec3 b, float t);
	gem::vec3 QuadraticLerp(gem::vec3 a, gem::vec3 b, gem::vec3 c, float t);
	gem::vec3 CubicLerp(gem::vec3 a, gem::vec3 b, gem::vec3 c, gem::vec3 d, float t);
};