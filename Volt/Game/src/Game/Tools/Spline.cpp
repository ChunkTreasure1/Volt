#include "gepch.h"
#include "Spline.h"

Spline::Spline(Ref<Volt::Scene>& aScene, int aAmoutOfCubes)
	:myCurrentScene(aScene)
{
	myAmountOfCubes = aAmoutOfCubes;
	myPointEntitys.reserve(4);
}

std::vector<gem::vec3> Spline::GetPointsInSegment(int index)
{
	std::vector<gem::vec3> points;
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
			std::vector<gem::vec3> points = GetPointsInSegment(i);
			float offset = 1.f / myBezierePoints[i].size();
			for (int n = 0; n < myBezierePoints[i].size(); n++)
			{
				gem::vec3 bezPos = CubicLerp(points[0], points[1], points[2], points[3], offset * n);
				myBezierePoints[i][n].GetComponent<Volt::TransformComponent>().position = bezPos;
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

	if (auto mesh = Volt::AssetManager::GetAsset<Volt::Mesh>("Assets/Meshes/Cube/cube.fbx"))
	{
		//comp0.handle = mesh->handle;
		//comp1.handle = mesh->handle;
		//comp2.handle = mesh->handle;
		//comp3.handle = mesh->handle;
	}

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

	if (auto mesh = Volt::AssetManager::GetAsset<Volt::Mesh>("Assets/Meshes/Primitives/Cube.vtmesh"))
	{
		comp0.handle = mesh->handle;
		comp1.handle = mesh->handle;
		comp2.handle = mesh->handle;
	}

	gem::vec3 anchPos = myPointEntitys[myPointEntitys.size() - 1].GetComponent<Volt::TransformComponent>().position - myPointEntitys[myPointEntitys.size() - 2].GetComponent<Volt::TransformComponent>().position;

	handlePoint0.GetComponent<Volt::TransformComponent>().position = myPointEntitys[myPointEntitys.size() - 1].GetComponent<Volt::TransformComponent>().position * 2 - myPointEntitys[myPointEntitys.size() - 2].GetComponent<Volt::TransformComponent>().position;
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

gem::vec3 Spline::Lerp(gem::vec3 a, gem::vec3 b, float t)
{
	return a + (b - a) * t;
}

gem::vec3 Spline::QuadraticLerp(gem::vec3 a, gem::vec3 b, gem::vec3 c, float t)
{
	gem::vec3 p1 = Lerp(a, b, t);
	gem::vec3 p2 = Lerp(b, c, t);
	return Lerp(p1, p2, t);
}

gem::vec3 Spline::CubicLerp(gem::vec3 a, gem::vec3 b, gem::vec3 c, gem::vec3 d, float t)
{
	gem::vec3 p1 = QuadraticLerp(a, b, c, t);
	gem::vec3 p2 = QuadraticLerp(b, c, d, t);

	return Lerp(p1, p2, t);
}
