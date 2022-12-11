#include "vtpch.h"
#include "Director.h"

#include <Volt/Components/Components.h>
#include <Volt/Components/VTCinemaComponents.h>

VT_REGISTER_SCRIPT(Director)

Director::Director(const Volt::Entity& aEntity)
	:ScriptBase(aEntity)
{
	if (!myInstance) 
	{
		myInstance = CreateRef<Director>(this);
	}
}

void Director::OnAwake()
{
	std::vector<Wire::EntityId> camEntIDs = myEntity.GetScene()->GetAllEntitiesWith<Volt::VTCamComponent>();

	myVTCams.resize(camEntIDs.size());

	for (auto ID : camEntIDs)
	{
		Volt::Entity ent = { ID, myEntity.GetScene() };

		if (ent.GetComponent<Volt::VTCamComponent>().isDefault)
			myActiveCamera = ent;
		
		myVTCams.emplace_back(ent);
	}
}

void Director::OnUpdate(float aDeltaTime)
{
	if (myChangeCamera) 
	{
		InitTransitionCam();
	}
}

void Director::InitTransitionCam()
{
}

void Director::SetActiveCamera(std::string aCamName)
{
	for (auto cam : myVTCams)
	{
		if (cam.GetComponent<Volt::TagComponent>().tag == aCamName)
		{
			myActiveCamera = cam;
			myChangeCamera = true;
			return;
		}
	}

	VT_CORE_ERROR(aCamName + ", does not exist!");
}

void Director::SetActiveCamera(size_t aIndex)
{
	for (auto cam : myVTCams)
	{
		cam.GetComponent<Volt::CameraComponent>().priority = 1;
	}

	myActiveCamera = myVTCams[aIndex];
	myActiveCamera.GetComponent<Volt::CameraComponent>().priority = 0;
}
