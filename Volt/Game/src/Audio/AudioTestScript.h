#pragma once
#include <Volt/Scripting/Script.h>
#include <Volt/Events/KeyEvent.h>

class AudioTestScript : public Volt::Script
{
public:
	AudioTestScript(Volt::Entity entity);
	~AudioTestScript() override = default;

	//void OnAwake() override;
	//void OnDetach() override;
	//void OnStart() override;
	//void OnStop() override;
	//void OnUpdate(float aDeltaTime) override;
	void OnEvent(Volt::Event& e) override;
	bool OnKeyEvent(Volt::KeyPressedEvent& e);

	void OnUpdate(float aDeltaTime) override;

	static WireGUID GetStaticGUID() { return "{586FF6D8-6733-4AE6-BCC5-7A8B3BA35774}"_guid; }
	WireGUID GetGUID() override { return GetStaticGUID(); }

private:
	float moveSpeed = 200.0f;
	float rotationSpeed = 5.0f;
	int forwardVelocity = 0;
	int strafeVeloctiy = 0;
	int rotationVeloctiy = 0;

};