#include "vtpch.h"
#include "Spline.h"
#include "Volt/Rendering/DebugRenderer.h"

Spline::Spline(Ref<Volt::Scene>& aScene, int aAmoutOfCubes)
	:myCurrentScene(aScene)
{
	myAmountOfCubes = aAmoutOfCubes;
	myPointEntitys.reserve(4);
}

std::vector<glm::vec3> Spline::GetPointsInSegment(int index)
{
	std::vector<glm::vec3> points;
	points.push_back(myPointEntitys[index * 3].GetComponent<Volt::TransformComponent>().position);
	points.push_back(myPointEntitys[index * 3 + 1].GetComponent<Volt::TransformComponent>().position);
	points.push_back(myPointEntitys[index * 3 + 2].GetComponent<Volt::TransformComponent>().position);
	points.push_back(myPointEntitys[index * 3 + 3].GetComponent<Volt::TransformComponent>().position);

	return points;
}

void Spline::UpdateSpline()
{
	if (myBezierePoints.size() != 0)
	{
		for (int i = 0; i < GetNumSegments(); i++)
		{
			std::vector<glm::vec3> points = GetPointsInSegment(i);
			float offset = 1.f / myBezierePoints[i].size();
			for (int n = 0; n < myBezierePoints[i].size(); n++)
			{
				glm::vec3 bezPos = CubicLerp(points[0], points[1], points[2], points[3], offset * n);
				myBezierePoints[i][n].GetComponent<Volt::TransformComponent>().position = bezPos;

				if (n + 1 < myBezierePoints[i].size())
				{
					Volt::DebugRenderer::DrawLine(myBezierePoints[i][n].GetPosition(), myBezierePoints[i][n + 1].GetPosition(), { 1,0,0,1 });
				}
			}
		}
	}
}

void Spline::InitSpline()
{
	myMainParentSpline = myCurrentScene->CreateEntity();
	myBezierPointParent = myCurrentScene->CreateEntity();

	myMainParentSpline.GetComponent<Volt::TagComponent>().tag = "Spline";
	myBezierPointParent.GetComponent<Volt::TagComponent>().tag = "BezierPoints";

	Volt::Entity anchorPoint0 = myCurrentScene->CreateEntity();
	auto& comp0 = anchorPoint0.AddComponent<Volt::MeshComponent>();
	Volt::Entity anchorPoint1 = myCurrentScene->CreateEntity();
	auto& comp1 = anchorPoint1.AddComponent<Volt::MeshComponent>();
	Volt::Entity handlePoint0 = myCurrentScene->CreateEntity();
	auto& comp2 = handlePoint0.AddComponent<Volt::MeshComponent>();
	Volt::Entity handlePoint1 = myCurrentScene->CreateEntity();
	auto& comp3 = handlePoint1.AddComponent<Volt::MeshComponent>();

	handlePoint0.GetComponent<Volt::TagComponent>().tag = "Handle";
	handlePoint1.GetComponent<Volt::TagComponent>().tag = "Handle";
	anchorPoint0.GetComponent<Volt::TagComponent>().tag = "Anchor";
	anchorPoint1.GetComponent<Volt::TagComponent>().tag = "Anchor";

	CreateNewBezierPoints();

	myPointEntitys.push_back(anchorPoint0);
	myPointEntitys.push_back(handlePoint0);
	myPointEntitys.push_back(handlePoint1);
	myPointEntitys.push_back(anchorPoint1);

	myCurrentScene->ParentEntity(myMainParentSpline, anchorPoint0);
	myCurrentScene->ParentEntity(myMainParentSpline, anchorPoint1);
	myCurrentScene->ParentEntity(myMainParentSpline, handlePoint0);
	myCurrentScene->ParentEntity(myMainParentSpline, handlePoint1);

	myCurrentScene->ParentEntity(anchorPoint0, handlePoint0);
	myCurrentScene->ParentEntity(anchorPoint1, handlePoint1);
}

void Spline::AddSegment()
{
	Volt::Entity handlePoint0 = myCurrentScene->CreateEntity();
	auto& comp0 = handlePoint0.AddComponent<Volt::MeshComponent>();
	Volt::Entity handlePoint1 = myCurrentScene->CreateEntity();
	auto& comp1 = handlePoint1.AddComponent<Volt::MeshComponent>();
	Volt::Entity anchorPoint = myCurrentScene->CreateEntity();
	auto& comp2 = anchorPoint.AddComponent<Volt::MeshComponent>();

	handlePoint0.GetComponent<Volt::TagComponent>().tag = "Handle";
	handlePoint1.GetComponent<Volt::TagComponent>().tag = "Handle";
	anchorPoint.GetComponent<Volt::TagComponent>().tag = "Anchor";

	glm::vec3 anchPos = myPointEntitys[myPointEntitys.size() - 1].GetComponent<Volt::TransformComponent>().position - myPointEntitys[myPointEntitys.size() - 2].GetComponent<Volt::TransformComponent>().position;

	handlePoint0.GetComponent<Volt::TransformComponent>().position = myPointEntitys[myPointEntitys.size() - 1].GetComponent<Volt::TransformComponent>().position * 2.f - myPointEntitys[myPointEntitys.size() - 2].GetComponent<Volt::TransformComponent>().position;
	handlePoint1.GetComponent<Volt::TransformComponent>().position = (myPointEntitys[myPointEntitys.size() - 1].GetComponent<Volt::TransformComponent>().position + myPointEntitys[myPointEntitys.size() - 1].GetComponent<Volt::TransformComponent>().position) * 0.5f;
	anchorPoint.GetComponent<Volt::TransformComponent>().position = myPointEntitys[myPointEntitys.size() - 1].GetComponent<Volt::TransformComponent>().position + anchPos;

	CreateNewBezierPoints();

	myCurrentScene->ParentEntity(myMainParentSpline, anchorPoint);
	myCurrentScene->ParentEntity(myMainParentSpline, handlePoint0);
	myCurrentScene->ParentEntity(myMainParentSpline, handlePoint1);

	myCurrentScene->ParentEntity(myPointEntitys[myPointEntitys.size() - 1], handlePoint0);
	myCurrentScene->ParentEntity(anchorPoint, handlePoint1);

	myPointEntitys.push_back(handlePoint0);
	myPointEntitys.push_back(handlePoint1);
	myPointEntitys.push_back(anchorPoint);
}

void Spline::CreateNewBezierPoints()
{
	std::vector<Volt::Entity> bezPoints;
	float offset = 1.f / 100.f;
	for (int n = 0; n < myAmountOfCubes; n++)
	{
		Volt::Entity point = myCurrentScene->CreateEntity();
		point.GetComponent<Volt::TagComponent>().tag = "BezierPoint" + std::to_string(n);
		auto& comp = point.AddComponent<Volt::MeshComponent>();

		bezPoints.push_back(point);
		myCurrentScene->ParentEntity(myBezierPointParent, point);
	}
	myBezierePoints.push_back(bezPoints);
}

glm::vec3 Spline::Lerp(glm::vec3 a, glm::vec3 b, float t)
{
	return a + (b - a) * t;
}

glm::vec3 Spline::QuadraticLerp(glm::vec3 a, glm::vec3 b, glm::vec3 c, float t)
{
	glm::vec3 p1 = Lerp(a, b, t);
	glm::vec3 p2 = Lerp(b, c, t);
	return Lerp(p1, p2, t);
}

glm::vec3 Spline::CubicLerp(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, float t)
{
	glm::vec3 p1 = QuadraticLerp(a, b, c, t);
	glm::vec3 p2 = QuadraticLerp(b, c, d, t);

	return Lerp(p1, p2, t);
}
