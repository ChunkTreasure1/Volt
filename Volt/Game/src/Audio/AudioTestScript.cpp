#include "gepch.h"
#include "AudioTestScript.h"
#include <Volt/Core/Application.h>
#include <Volt/Events/KeyEvent.h>
#include <Volt/Input/KeyCodes.h>

#include <Audio/AudioSource.h>

#include <Volt/Scripting/ScriptRegistry.h>

VT_REGISTER_SCRIPT(AudioTestScript);
AudioTestScript::AudioTestScript(Volt::Entity entity) : Volt::ScriptBase(entity) {}

void AudioTestScript::OnEvent(Volt::Event& e)
{
	Volt::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Volt::KeyPressedEvent>(VT_BIND_EVENT_FN(AudioTestScript::OnKeyEvent));
}

bool AudioTestScript::OnKeyEvent(Volt::KeyPressedEvent& e)
{
	if (e.GetKeyCode() == VT_KEY_W)
	{
		forwardVelocity = 1;
	}
	else if (e.GetKeyCode() == VT_KEY_S)
	{
		forwardVelocity = -1;
	}
	if (e.GetKeyCode() == VT_KEY_D)
	{
		strafeVeloctiy = 1;
	}
	else if (e.GetKeyCode() == VT_KEY_A)
	{
		strafeVeloctiy = -1;
	}

	if (e.GetKeyCode() == VT_KEY_E)
	{
		rotationVeloctiy = 1;
	}
	else if (e.GetKeyCode() == VT_KEY_Q)
	{
		rotationVeloctiy = -1;
	}

	AudioSourceScript* audioSource = myEntity.GetScript<AudioSourceScript>();

	if (e.GetKeyCode() == VT_KEY_1)
	{
		audioSource->Play("event:/TestDoot2D");
	}
	if (e.GetKeyCode() == VT_KEY_2)
	{
		audioSource->Play("event:/TestDoot3D");
	}
	if (e.GetKeyCode() == VT_KEY_3)
	{
		audioSource->Play("event:/TestTone2DLoop");
	}
	if (e.GetKeyCode() == VT_KEY_4)
	{
		audioSource->Play("event:/TestTone3DLoop");
	}

	return false;
}

void AudioTestScript::OnUpdate(float aDeltaTime)
{
	gem::vec3 position = myEntity.GetPosition();
	position.x += moveSpeed * strafeVeloctiy * aDeltaTime;
	position.z += moveSpeed * forwardVelocity * aDeltaTime;
	myEntity.SetPosition(position);

	gem::vec3 rotation = myEntity.GetRotation();
	rotation.y += rotationSpeed * rotationVeloctiy * aDeltaTime;

	myEntity.SetRotation(rotation);

	gem::vec3 curRotation = myEntity.GetRotation();
	if (curRotation.y > 3.1415f)
	{
		curRotation.y = -3.1415f;
	}
	else if (curRotation.y < -3.1415f)
	{
		curRotation.y = 3.1415f;
	}
	myEntity.SetRotation(curRotation);

	forwardVelocity = 0;
	strafeVeloctiy = 0;
	rotationVeloctiy = 0;

}
