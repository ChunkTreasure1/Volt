#pragma once

#include "Sandbox/Window/EditorWindow.h"

#include "Volt/AI/NavMesh.h"
#include "Volt/AI/NavigationSystem.h"
#include "Volt/Scene/Scene.h"

#include "Sandbox/Navigation/NMConverter.h"
#include "Sandbox/Navigation/NavMeshBuilder.h"
#include "Sandbox/Navigation/ext/MeshLoaderObj.h"

class NavigationPanel : public EditorWindow
{
public:
	NavigationPanel(Ref<Volt::Scene>& currentScene) 
		: EditorWindow("Navigation"), myCurrentScene(currentScene) {}
	void UpdateMainContent() override;

private:
	bool CreateObjFile(Ref<Volt::Mesh> mesh, const std::string& path);

	void BakeTab();
	void ObjectsTab();
	void DebugTab();

	Volt::NavMeshBuilder myBuilder;
	InputGeom myGeometry;
	Ref<Volt::NavMesh> myVTNavMesh;

	Ref<Volt::Scene>& myCurrentScene;
};