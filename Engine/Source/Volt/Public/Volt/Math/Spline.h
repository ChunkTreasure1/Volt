#pragma once
#include <Volt/Core/Base.h>
#include <glm/glm.hpp>

#include <Volt/Scene/Scene.h>
#include "Volt/Scene/Entity.h"
#include <AssetSystem/AssetManager.h>
#include "Volt/Asset/Mesh/Mesh.h"

class Spline
{
public:
	Spline(Ref<Volt::Scene>& aScene, int aAmoutOfCubes);
	~Spline() = default;
	
	int GetNumSegments() { return ((int32_t)myPointEntitys.size() - 4) / 3 + 1; }
	Vector<glm::vec3> GetPointsInSegment(int index);

	void CreateNewBezierPoints();
	void UpdateSpline();
	void InitSpline();
	void AddSegment();

private:
	Vector<Volt::Entity> myPointEntitys;
	Vector<Vector<Volt::Entity>> myBezierePoints;

	Volt::Entity myMainParentSpline;
	Volt::Entity myBezierPointParent;

	int myAmountOfCubes;

	Ref<Volt::Scene>& myCurrentScene;

	glm::vec3 Lerp(glm::vec3 a, glm::vec3 b, float t);
	glm::vec3 QuadraticLerp(glm::vec3 a, glm::vec3 b, glm::vec3 c, float t);
	glm::vec3 CubicLerp(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, float t);
};
