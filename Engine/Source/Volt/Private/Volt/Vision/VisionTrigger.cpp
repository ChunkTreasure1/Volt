#include "vtpch.h"
////#include "VisionTrigger.h"
////
////#include <Volt/Vision/Vision.h>
////#include <Volt/Vision/VisionComponents.h>
////
////VT_REGISTER_SCRIPT(VisionTrigger)
////
////VisionTrigger::VisionTrigger(const Volt::Entity& aEntity)
////	:	Script(aEntity)
////{
////}
////
////void VisionTrigger::OnAwake()
////{
////	if (myEntity.HasComponent<Volt::VisionTriggerComponent>())
////	{
////		const auto& triggerComp = myEntity.GetComponent<Volt::VisionTriggerComponent>();
////
////		myTriggerCam = Volt::Entity{ triggerComp.triggerCam, myEntity.GetScene() };
////	}
////}
////
////void VisionTrigger::OnUpdate(float aDeltaTime)
////{
////}
////
////void VisionTrigger::OnTriggerEnter(Volt::Entity entity, bool isTrigger)
////{
////	if (!myTriggerCam.IsNull())
////	{
////		auto& director = Volt::Vision::Get();
////
////		if (director.IsTriggerCamActive())
////		{
////			if (myTriggerCam = director.GetActiveCamera()) 
////			{
////				return;
////			}
////		}
////		else 
////		{
////			director.SetLastActiveCamera(director.GetActiveCamera());
////		}
////
////
////		director.SetTriggerCamera(myTriggerCam);
////		director.SetTriggerCamActive(true);
////	}
////}
////
////void VisionTrigger::OnTriggerExit(Volt::Entity entity, bool isTrigger)
////{
////	if (!myTriggerCam.IsNull())
////	{
////		auto& director = Volt::Vision::Get();
////
////		if (myTriggerCam == director.GetTriggerCamera())
////		{
////			director.SetTriggerCamActive(false);
////			director.SetActiveCamera(director.GetLastActiveCamera());
////		}
////	}
////}
